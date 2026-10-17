#include "ghostwire/identity/identity_service.hpp"
#include <algorithm>
#include <atomic>
#include <limits>
#include <new>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>

namespace ghostwire::identity {
namespace {
[[noreturn]] void fail(ErrorCode code) {
    throw Error{code};
}
bool valid_alias(std::string_view text) noexcept {
    return !text.empty() && text.size() <= 20 &&
           std::all_of(text.begin(), text.end(), [](char c) { return c >= '0' && c <= '9'; });
}
bool valid_name(std::string_view text) noexcept {
    if (text.size() > 256)
        return false;
    std::size_t at = 0;
    while (at < text.size()) {
        const auto first = static_cast<unsigned char>(text[at++]);
        std::uint32_t code = first;
        std::size_t remaining = 0;
        std::uint32_t minimum = 0;
        if (first >= 0xC2 && first <= 0xDF) {
            remaining = 1;
            code = first & 0x1FU;
            minimum = 0x80;
        } else if (first >= 0xE0 && first <= 0xEF) {
            remaining = 2;
            code = first & 0x0FU;
            minimum = 0x800;
        } else if (first >= 0xF0 && first <= 0xF4) {
            remaining = 3;
            code = first & 0x07U;
            minimum = 0x10000;
        } else if (first >= 0x80) {
            return false;
        }
        if (remaining > text.size() - at)
            return false;
        for (std::size_t i = 0; i < remaining; ++i) {
            const auto byte = static_cast<unsigned char>(text[at++]);
            if ((byte & 0xC0U) != 0x80U)
                return false;
            code = (code << 6U) | (byte & 0x3FU);
        }
        if (code < minimum || code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF) || code < 0x20 ||
            (code >= 0x7F && code <= 0x9F))
            return false;
    }
    return true;
}
Labels make_labels(std::string_view alias, std::string_view name) {
    if (!valid_alias(alias) || !valid_name(name))
        fail(ErrorCode::invalid_argument);
    return {std::string(alias), std::string(name)};
}
bool valid_limits(const ServiceLimits &limits) noexcept {
    return limits.profiles > 0 && limits.profiles <= 8 && limits.devices_per_authority > 0 &&
           limits.devices_per_authority <= maximum_devices && limits.memberships_per_profile <= 32 &&
           limits.private_keys >= 2 && limits.private_keys <= 128 && limits.trust_contexts <= 256 &&
           limits.public_record_bytes <= 2 * 1024 * 1024 && limits.input_bytes > 0 &&
           limits.input_bytes <= 8 * 1024 && limits.issued_lifetime_seconds > 0 &&
           limits.issued_lifetime_seconds <= limits.maximum_lifetime_seconds &&
           limits.maximum_lifetime_seconds <= maximum_device_set_lifetime &&
           limits.operation_timeout >= std::chrono::milliseconds(1) &&
           limits.operation_timeout <= std::chrono::milliseconds(5000);
}
void valid_expected(const ExpectedAuthority &authority) {
    if (authority.kind != AuthorityKind::profile && authority.kind != AuthorityKind::membership)
        fail(ErrorCode::wrong_scope);
    if (authority.kind == AuthorityKind::profile && authority.scope != CommunityId{})
        fail(ErrorCode::wrong_scope);
}
void valid_at(const DeviceSet &record, std::uint64_t now) {
    if (now < record.valid_from)
        fail(ErrorCode::not_yet_valid);
    if (now >= record.valid_until)
        fail(ErrorCode::expired);
}
} // namespace

namespace detail {
struct OwnerIdentity {};
struct DeviceOwner {
    crypto::SigningKey key;
    crypto::PublicKey public_key{};
    ProfileDeviceId profile_id{};
    MembershipDeviceId membership_id{};
};
struct AuthorityOwner {
    crypto::SigningKey key;
    DeviceSet record;
    DeviceSetFrame frame;
    std::array<DeviceOwner, maximum_devices> devices;
    std::size_t device_count = 0;
};
struct Membership {
    MembershipId id;
    CommunityId community;
    Labels labels;
    AuthorityOwner authority;
};
struct Trust {
    ExpectedAuthority expected;
    DeviceSet record;
    DeviceSetFrame frame;
    bool verified = false, blocked = false, conflict = false;
};
template <class T> struct Slot {
    std::uint64_t generation = 1;
    std::unique_ptr<T> value;
};
struct CommunityPin {
    CommunityId id;
    crypto::PublicKey key{};
};
struct Profile {
    ProfileIdentityId id;
    Labels labels;
    AuthorityOwner authority;
    std::vector<Slot<Membership>> memberships;
    std::vector<Slot<Trust>> trusts;
    std::vector<CommunityPin> communities;
};

struct StopForwarder {
    std::stop_source *destination;
    void operator()() const noexcept { destination->request_stop(); }
};
struct Operation {
    const core::Clock &clock;
    core::Clock::TimePoint started;
    core::Clock::Duration timeout;
    std::stop_source combined;
    std::stop_callback<StopForwarder> owned_callback, application_callback, call_callback;
    Operation(const core::Clock &value, std::chrono::milliseconds duration, std::stop_token owned,
              std::stop_token application, std::stop_token call)
        : clock(value), started(value.now()), timeout(duration),
          owned_callback(owned, StopForwarder{&combined}),
          application_callback(application, StopForwarder{&combined}),
          call_callback(call, StopForwarder{&combined}) {}
    [[nodiscard]] std::stop_token token() const noexcept { return combined.get_token(); }
    void check() const {
        if (combined.stop_requested())
            fail(ErrorCode::cancelled);
        const auto now = clock.now();
        if (now < started || started > core::Clock::TimePoint::max() - timeout || now >= started + timeout)
            fail(ErrorCode::deadline_exceeded);
    }
    template <class T> T take(crypto::Result<T> result) const {
        if (!result)
            throw from_crypto_error(result.error());
        check();
        return std::move(result).value();
    }
    template <class T> T take(Result<T> result) const {
        if (!result)
            throw result.error();
        check();
        return std::move(result).value();
    }
};

struct ServiceState {
    crypto::CryptoProvider &provider;
    const core::Clock &clock;
    ServiceLimits limits;
    std::stop_token application_stop;
    std::stop_source stop;
    std::atomic_flag in_progress = ATOMIC_FLAG_INIT;
    std::thread::id owner_thread = std::this_thread::get_id();
    std::shared_ptr<const OwnerIdentity> owner = std::make_shared<OwnerIdentity>();
    std::vector<Slot<Profile>> profiles;
    identity::ServiceState lifecycle = identity::ServiceState::running;
    std::optional<std::uint64_t> largest_utc;
    bool time_requires_revalidation = false;

    ServiceState(crypto::CryptoProvider &crypto, const core::Clock &monotonic, ServiceLimits bounds,
                 std::stop_token application)
        : provider(crypto), clock(monotonic), limits(bounds), application_stop(application),
          profiles(bounds.profiles) {}

    template <class T, class Function> Result<T> run(std::stop_token call, Function &&function) {
        if (in_progress.test_and_set(std::memory_order_acquire))
            return Error{ErrorCode::busy};
        struct Release {
            std::atomic_flag &flag;
            ~Release() { flag.clear(std::memory_order_release); }
        } release{in_progress};
        try {
            if (owner_thread != std::this_thread::get_id() || lifecycle != identity::ServiceState::running)
                fail(ErrorCode::invalid_state);
            Operation operation(clock, limits.operation_timeout, stop.get_token(), application_stop, call);
            operation.check();
            if constexpr (std::is_void_v<T>) {
                function(operation);
                return {};
            } else {
                return function(operation);
            }
        } catch (const Error &error) {
            return error;
        } catch (const std::bad_alloc &) {
            return Error{ErrorCode::resource_exhausted};
        } catch (...) {
            return Error{ErrorCode::crypto_failure};
        }
    }
    template <class Tag>
    LocalHandle<Tag> handle(std::size_t slot, std::uint64_t generation, std::size_t parent = 0,
                            std::uint64_t parent_generation = 0) {
        LocalHandle<Tag> result;
        result.owner_ = owner;
        result.slot_ = slot;
        result.generation_ = generation;
        result.parent_ = parent;
        result.parent_generation_ = parent_generation;
        return result;
    }
    Profile &get(ProfileHandle value) {
        if (value.owner_ != owner || value.slot_ >= profiles.size() || value.generation_ == 0 ||
            profiles[value.slot_].generation != value.generation_ || !profiles[value.slot_].value)
            fail(ErrorCode::invalid_handle);
        return *profiles[value.slot_].value;
    }
    Membership &get(ProfileHandle profile, MembershipHandle value) {
        auto &parent = get(profile);
        if (value.owner_ != owner || value.parent_ != profile.slot_ ||
            value.parent_generation_ != profile.generation_ || value.slot_ >= parent.memberships.size() ||
            value.generation_ == 0 || parent.memberships[value.slot_].generation != value.generation_ ||
            !parent.memberships[value.slot_].value)
            fail(ErrorCode::invalid_handle);
        return *parent.memberships[value.slot_].value;
    }
    Trust &get(ProfileHandle profile, TrustHandle value) {
        auto &parent = get(profile);
        if (value.owner_ != owner || value.parent_ != profile.slot_ ||
            value.parent_generation_ != profile.generation_ || value.slot_ >= parent.trusts.size() ||
            value.generation_ == 0 || parent.trusts[value.slot_].generation != value.generation_ ||
            !parent.trusts[value.slot_].value)
            fail(ErrorCode::invalid_handle);
        return *parent.trusts[value.slot_].value;
    }
    template <class T> std::size_t free_slot(const std::vector<Slot<T>> &slots) const {
        for (std::size_t i = 0; i < slots.size(); ++i)
            if (!slots[i].value && slots[i].generation != std::numeric_limits<std::uint64_t>::max())
                return i;
        fail(ErrorCode::resource_exhausted);
    }
    template <class T> void erase(Slot<T> &slot) noexcept {
        slot.value.reset();
        if (slot.generation != std::numeric_limits<std::uint64_t>::max())
            ++slot.generation;
        // Exhausted generations retire the slot forever; old handles cannot revive.
    }
    [[nodiscard]] ServiceDiagnostic diagnostic() const noexcept {
        ServiceDiagnostic result;
        result.state = lifecycle;
        for (const auto &slot : profiles) {
            if (!slot.value)
                continue;
            const auto &profile = *slot.value;
            ++result.profiles;
            result.devices += profile.authority.device_count;
            result.private_keys += 1 + profile.authority.device_count;
            result.public_record_bytes += profile.authority.frame.size;
            result.trust_contexts += profile.communities.size();
            for (const auto &member_slot : profile.memberships) {
                if (!member_slot.value)
                    continue;
                const auto &member = *member_slot.value;
                ++result.memberships;
                result.devices += member.authority.device_count;
                result.private_keys += 1 + member.authority.device_count;
                result.public_record_bytes += member.authority.frame.size;
            }
            for (const auto &trust_slot : profile.trusts) {
                if (!trust_slot.value)
                    continue;
                ++result.trust_contexts;
                result.public_record_bytes += trust_slot.value->frame.size;
            }
        }
        return result;
    }
    void keys_available(std::size_t additional) const {
        const auto used = diagnostic().private_keys;
        if (used > limits.private_keys || additional > limits.private_keys - used)
            fail(ErrorCode::resource_exhausted);
    }
    void bytes_available(std::size_t replaced, std::size_t additional) const {
        const auto used = diagnostic().public_record_bytes;
        if (replaced > used || used - replaced > limits.public_record_bytes ||
            additional > limits.public_record_bytes - (used - replaced))
            fail(ErrorCode::resource_exhausted);
    }
    void trust_available() const {
        if (diagnostic().trust_contexts >= limits.trust_contexts)
            fail(ErrorCode::resource_exhausted);
    }
    std::uint64_t time(VerificationTime supplied) {
        if (time_requires_revalidation || !supplied.trusted || supplied.seconds > maximum_utc_seconds)
            fail(ErrorCode::time_untrusted);
        if (largest_utc && supplied.seconds < *largest_utc && *largest_utc - supplied.seconds > 5) {
            time_requires_revalidation = true;
            fail(ErrorCode::time_untrusted);
        }
        const auto value = largest_utc ? std::max(*largest_utc, supplied.seconds) : supplied.seconds;
        largest_utc = value;
        return value;
    }
    bool key_in_use(const crypto::PublicKey &key) const noexcept {
        const auto matches = [&](const AuthorityOwner &authority) {
            if (authority.record.authority.public_key == key)
                return true;
            for (std::size_t i = 0; i < authority.device_count; ++i)
                if (authority.devices[i].public_key == key)
                    return true;
            return false;
        };
        for (const auto &slot : profiles) {
            if (!slot.value)
                continue;
            if (matches(slot.value->authority))
                return true;
            for (const auto &pin : slot.value->communities)
                if (pin.key == key)
                    return true;
            for (const auto &trust : slot.value->trusts) {
                if (!trust.value)
                    continue;
                if (trust.value->expected.public_key == key)
                    return true;
                for (std::size_t i = 0; i < trust.value->record.device_count; ++i)
                    if (trust.value->record.devices[i].public_key == key)
                        return true;
            }
            for (const auto &member : slot.value->memberships)
                if (member.value && matches(member.value->authority))
                    return true;
        }
        return false;
    }
    void check_device_id(const DeviceOwner &device, AuthorityKind kind) const {
        const auto check = [&](const AuthorityOwner &authority) {
            if (authority.record.authority.kind != kind)
                return;
            for (std::size_t i = 0; i < authority.device_count; ++i) {
                const auto &existing = authority.devices[i];
                if ((kind == AuthorityKind::profile && existing.profile_id == device.profile_id) ||
                    (kind == AuthorityKind::membership && existing.membership_id == device.membership_id))
                    fail(ErrorCode::crypto_failure);
            }
        };
        for (const auto &slot : profiles) {
            if (!slot.value)
                continue;
            check(slot.value->authority);
            for (const auto &member : slot.value->memberships)
                if (member.value)
                    check(member.value->authority);
        }
    }
    DeviceOwner generate(Operation &operation) {
        operation.check();
        DeviceOwner result;
        result.key = operation.take(provider.generate_signing_key(operation.token()));
        result.public_key = operation.take(provider.public_key(result.key, operation.token()));
        if (key_in_use(result.public_key))
            fail(ErrorCode::crypto_failure);
        return result;
    }
    DeviceSet next_record(const AuthorityOwner &owner_value, std::uint64_t now) const {
        if (owner_value.record.revision == std::numeric_limits<std::uint64_t>::max())
            fail(ErrorCode::revision_exhausted);
        if (now > maximum_utc_seconds - limits.issued_lifetime_seconds)
            fail(ErrorCode::time_untrusted);
        DeviceSet record;
        record.authority = owner_value.record.authority;
        record.revision = owner_value.record.revision + 1;
        record.valid_from = now;
        record.valid_until = now + limits.issued_lifetime_seconds;
        return record;
    }
    DeviceSetFrame sign(AuthorityOwner &owner_value, DeviceSet &record, Operation &operation) {
        std::sort(record.devices.begin(),
                  record.devices.begin() + static_cast<std::ptrdiff_t>(record.device_count),
                  [](const DeviceEntry &left, const DeviceEntry &right) {
                      return left.public_key < right.public_key;
                  });
        const auto bytes =
            device_set_header_bytes + crypto::signature_bytes + record.device_count * device_set_entry_bytes;
        bytes_available(owner_value.frame.size, bytes);
        operation.check();
        return operation.take(sign_device_set(provider, owner_value.key, record, operation.token()));
    }
    void initial_record(AuthorityOwner &authority, std::uint64_t now, Operation &operation) {
        auto record = next_record(authority, now);
        record.device_count = 1;
        record.devices[0] = {authority.devices[0].public_key, record.valid_from, record.valid_until};
        auto frame = sign(authority, record, operation);
        authority.record = record;
        authority.frame = frame;
        authority.device_count = 1;
    }
    ProfileSnapshot snapshot(ProfileHandle handle_value, const Profile &profile) const {
        ProfileSnapshot result{handle_value,   profile.id, profile.authority.record.authority.public_key,
                               profile.labels, {},         profile.authority.record.revision};
        result.devices.reserve(profile.authority.device_count);
        for (std::size_t i = 0; i < profile.authority.device_count; ++i)
            result.devices.push_back(
                {profile.authority.devices[i].profile_id, profile.authority.devices[i].public_key});
        return result;
    }
    MembershipSnapshot snapshot(MembershipHandle handle_value, const Membership &member) const {
        MembershipSnapshot result{handle_value,
                                  member.id,
                                  member.community,
                                  member.authority.record.authority.public_key,
                                  member.labels,
                                  {},
                                  member.authority.record.revision};
        result.devices.reserve(member.authority.device_count);
        for (std::size_t i = 0; i < member.authority.device_count; ++i)
            result.devices.push_back(
                {member.authority.devices[i].membership_id, member.authority.devices[i].public_key});
        return result;
    }
    // A key already known locally in an incompatible authority/device role is
    // never reclassified by a public record. Public keys convey no ownership.
    void check_roles(const DeviceSet &record, const Profile *context = nullptr) const {
        const auto check_known = [&](const DeviceSet &known) {
            const auto &expected = known.authority;
            if (record.authority.public_key == expected.public_key && record.authority != expected)
                fail(ErrorCode::wrong_scope);
            for (std::size_t i = 0; i < known.device_count; ++i)
                if (record.authority.public_key == known.devices[i].public_key)
                    fail(ErrorCode::wrong_scope);
            for (std::size_t i = 0; i < record.device_count; ++i) {
                if (record.devices[i].public_key == expected.public_key)
                    fail(ErrorCode::wrong_scope);
                for (std::size_t j = 0; j < known.device_count; ++j)
                    if (record.devices[i].public_key == known.devices[j].public_key &&
                        record.authority != expected)
                        fail(ErrorCode::wrong_scope);
            }
        };
        for (std::size_t i = 0; i < record.device_count; ++i)
            if (record.devices[i].public_key == record.authority.public_key)
                fail(ErrorCode::wrong_scope);
        for (const auto &slot : profiles) {
            if (!slot.value)
                continue;
            check_known(slot.value->authority.record);
            for (const auto &member : slot.value->memberships)
                if (member.value)
                    check_known(member.value->authority.record);
        }
        if (context) {
            for (const auto &slot : context->trusts) {
                if (!slot.value)
                    continue;
                auto known = slot.value->record;
                known.authority = slot.value->expected;
                check_known(known);
            }
            for (const auto &pin : context->communities) {
                if (pin.key == record.authority.public_key)
                    fail(ErrorCode::wrong_scope);
                for (std::size_t i = 0; i < record.device_count; ++i)
                    if (pin.key == record.devices[i].public_key)
                        fail(ErrorCode::wrong_scope);
            }
        }
    }
    const DeviceSet *owned_record(const Profile &profile, const ExpectedAuthority &expected) const noexcept {
        if (profile.authority.record.authority == expected)
            return &profile.authority.record;
        for (const auto &slot : profile.memberships)
            if (slot.value && slot.value->authority.record.authority == expected)
                return &slot.value->authority.record;
        return nullptr;
    }
    void recognized_by(const DeviceSet &record, const crypto::PublicKey &key, std::uint64_t now) const {
        valid_at(record, now);
        for (std::size_t i = 0; i < record.device_count; ++i) {
            const auto &device = record.devices[i];
            if (device.public_key != key)
                continue;
            if (now < device.valid_from)
                fail(ErrorCode::not_yet_valid);
            if (now >= device.valid_until)
                fail(ErrorCode::expired);
            return;
        }
        fail(ErrorCode::unauthorized_device);
    }
    enum class Change { add, rotate, revoke, renew };
    DeviceOwner change(AuthorityOwner &authority, Change change_kind, std::size_t index,
                       ProfileIdentityId profile_id, MembershipId membership_id, VerificationTime supplied,
                       Operation &operation) {
        const bool creates = change_kind == Change::add || change_kind == Change::rotate;
        if (change_kind == Change::add && authority.device_count >= limits.devices_per_authority)
            fail(ErrorCode::resource_exhausted);
        if ((change_kind == Change::rotate || change_kind == Change::revoke) &&
            index >= authority.device_count)
            fail(ErrorCode::unauthorized_device);
        if (creates)
            keys_available(1); // Includes the staged replacement before old key release.
        const auto now = time(supplied);
        auto record = next_record(authority, now);
        DeviceOwner fresh;
        if (creates) {
            fresh = generate(operation);
            if (authority.record.authority.kind == AuthorityKind::profile)
                fresh.profile_id = operation.take(
                    derive_profile_device_id(provider, profile_id, fresh.public_key, operation.token()));
            else
                fresh.membership_id = operation.take(derive_membership_device_id(
                    provider, membership_id, fresh.public_key, operation.token()));
            check_device_id(fresh, authority.record.authority.kind);
        }
        for (std::size_t i = 0; i < authority.device_count; ++i) {
            if ((change_kind == Change::rotate || change_kind == Change::revoke) && i == index)
                continue;
            record.devices[record.device_count++] = {authority.devices[i].public_key, record.valid_from,
                                                     record.valid_until};
        }
        if (creates)
            record.devices[record.device_count++] = {fresh.public_key, record.valid_from, record.valid_until};
        auto frame = sign(authority, record, operation);
        DeviceOwner result;
        result.public_key = fresh.public_key;
        result.profile_id = fresh.profile_id;
        result.membership_id = fresh.membership_id;
        operation.check(); // The only publication point. Everything below is noexcept.
        if (change_kind == Change::add)
            authority.devices[authority.device_count++] = std::move(fresh);
        else if (change_kind == Change::rotate)
            authority.devices[index] = std::move(fresh);
        else if (change_kind == Change::revoke) {
            for (std::size_t i = index + 1; i < authority.device_count; ++i)
                authority.devices[i - 1] = std::move(authority.devices[i]);
            authority.devices[--authority.device_count] = DeviceOwner{};
        }
        authority.record = record;
        authority.frame = frame;
        return result;
    }
    void accept(const Profile &profile, Trust &trust, std::span<const std::uint8_t> bytes,
                VerificationTime supplied, bool resolving, Operation &operation) {
        if (trust.blocked && !resolving)
            fail(ErrorCode::blocked);
        if (trust.conflict && !resolving)
            fail(ErrorCode::conflicting_revision);
        if (resolving && !trust.conflict)
            fail(ErrorCode::invalid_state);
        if (bytes.size() > limits.input_bytes)
            fail(ErrorCode::malformed_record);
        const auto now = time(supplied);
        auto record = operation.take(verify_device_set(provider, bytes, trust.expected, {now, true},
                                                       limits.maximum_lifetime_seconds, operation.token()));
        check_roles(record, &profile);
        if (record.revision < trust.record.revision)
            fail(ErrorCode::stale_revision);
        if (const auto *owned = owned_record(profile, trust.expected);
            owned && record.revision < owned->revision)
            fail(ErrorCode::stale_revision);
        DeviceSetFrame frame;
        frame.size = bytes.size();
        std::copy(bytes.begin(), bytes.end(), frame.bytes.begin());
        if (!resolving && record.revision == trust.record.revision && frame != trust.frame) {
            operation.check();
            trust.conflict = true; // Deliberate security state transition, no record replacement.
            fail(ErrorCode::conflicting_revision);
        }
        bytes_available(trust.frame.size, frame.size);
        operation.check();
        trust.record = record;
        trust.frame = frame;
        trust.conflict = false;
    }
};
} // namespace detail

IdentityService::IdentityService(std::unique_ptr<detail::ServiceState> state) noexcept
    : state_(std::move(state)) {}
IdentityService::~IdentityService() noexcept = default;

Result<std::unique_ptr<IdentityService>> IdentityService::create(crypto::CryptoProvider &provider,
                                                                 const core::Clock &clock,
                                                                 ServiceLimits limits,
                                                                 std::stop_token application) {
    if (!valid_limits(limits))
        return Error{ErrorCode::invalid_argument};
    if (application.stop_requested())
        return Error{ErrorCode::cancelled};
    if (provider.diagnostic().stopped)
        return Error{ErrorCode::invalid_state};
    try {
        return std::unique_ptr<IdentityService>(new IdentityService(
            std::make_unique<detail::ServiceState>(provider, clock, limits, application)));
    } catch (const std::bad_alloc &) {
        return Error{ErrorCode::resource_exhausted};
    } catch (...) {
        return Error{ErrorCode::crypto_failure};
    }
}
void IdentityService::request_stop() noexcept {
    state_->stop.request_stop();
}
Result<void> IdentityService::close() {
    if (state_->in_progress.test_and_set(std::memory_order_acquire))
        return Error{ErrorCode::busy};
    struct Release {
        std::atomic_flag &flag;
        ~Release() { flag.clear(std::memory_order_release); }
    } release{state_->in_progress};
    if (state_->owner_thread != std::this_thread::get_id())
        return Error{ErrorCode::invalid_state};
    state_->stop.request_stop();
    state_->lifecycle = ServiceState::stopping;
    for (auto &slot : state_->profiles)
        state_->erase(slot);
    state_->lifecycle = ServiceState::stopped;
    return {};
}
ServiceDiagnostic IdentityService::diagnostic() const noexcept {
    return state_->diagnostic();
}

Result<ProfileHandle> IdentityService::create_profile(std::string_view alias, std::string_view name,
                                                      VerificationTime supplied, std::stop_token stop) {
    return state_->run<ProfileHandle>(stop, [&](detail::Operation &operation) {
        auto labels = make_labels(alias, name);
        const auto slot = state_->free_slot(state_->profiles);
        state_->keys_available(2);
        state_->bytes_available(0,
                                device_set_header_bytes + crypto::signature_bytes + device_set_entry_bytes);
        const auto now = state_->time(supplied);
        auto profile = std::make_unique<detail::Profile>();
        profile->memberships.resize(state_->limits.memberships_per_profile);
        profile->trusts.resize(state_->limits.trust_contexts);
        profile->communities.reserve(state_->limits.trust_contexts);
        profile->labels = std::move(labels);
        auto root = state_->generate(operation);
        profile->authority.key = std::move(root.key);
        profile->authority.record.authority = {AuthorityKind::profile, {}, root.public_key};
        profile->id = operation.take(derive_profile_id(state_->provider, root.public_key, operation.token()));
        for (const auto &existing : state_->profiles)
            if (existing.value && existing.value->id == profile->id)
                fail(ErrorCode::crypto_failure);
        auto device = state_->generate(operation);
        if (device.public_key == root.public_key)
            fail(ErrorCode::crypto_failure);
        device.profile_id = operation.take(
            derive_profile_device_id(state_->provider, profile->id, device.public_key, operation.token()));
        state_->check_device_id(device, AuthorityKind::profile);
        profile->authority.devices[0] = std::move(device);
        state_->initial_record(profile->authority, now, operation);
        const auto handle = state_->handle<detail::ProfileTag>(slot, state_->profiles[slot].generation);
        operation.check();
        state_->profiles[slot].value = std::move(profile);
        return handle;
    });
}
Result<ProfileSnapshot> IdentityService::profile(ProfileHandle handle, std::stop_token stop) {
    return state_->run<ProfileSnapshot>(stop, [&](detail::Operation &operation) {
        auto result = state_->snapshot(handle, state_->get(handle));
        operation.check();
        return result;
    });
}
Result<std::vector<ProfileSnapshot>> IdentityService::profiles(std::stop_token stop) {
    return state_->run<std::vector<ProfileSnapshot>>(stop, [&](detail::Operation &operation) {
        std::vector<ProfileSnapshot> result;
        result.reserve(state_->diagnostic().profiles);
        for (std::size_t i = 0; i < state_->profiles.size(); ++i)
            if (state_->profiles[i].value)
                result.push_back(
                    state_->snapshot(state_->handle<detail::ProfileTag>(i, state_->profiles[i].generation),
                                     *state_->profiles[i].value));
        operation.check();
        return result;
    });
}
Result<void> IdentityService::rename_profile(ProfileHandle handle, std::string_view alias,
                                             std::string_view name, std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        auto &profile = state_->get(handle);
        auto labels = make_labels(alias, name);
        operation.check();
        profile.labels = std::move(labels);
    });
}
Result<void> IdentityService::delete_profile(ProfileHandle handle, std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        (void)state_->get(handle);
        operation.check();
        state_->erase(state_->profiles[handle.slot_]);
    });
}
Result<CommunityId> IdentityService::pin_community(ProfileHandle handle, const crypto::PublicKey &key,
                                                   std::stop_token stop) {
    return state_->run<CommunityId>(stop, [&](detail::Operation &operation) {
        auto &profile = state_->get(handle);
        const auto id = operation.take(derive_community_id(state_->provider, key, operation.token()));
        for (const auto &other_profile : state_->profiles)
            if (other_profile.value)
                for (const auto &pin : other_profile.value->communities)
                    if (pin.id == id && pin.key != key)
                        fail(ErrorCode::conflicting_revision);
        for (const auto &existing : profile.communities) {
            if (existing.id == id) {
                if (existing.key != key)
                    fail(ErrorCode::conflicting_revision);
                return id;
            }
        }
        state_->trust_available();
        // A community key may intentionally be pinned in several profiles. It
        // must not be an authority/device key already known in this profile.
        for (const auto &slot : profile.trusts) {
            if (!slot.value)
                continue;
            if (slot.value->expected.public_key == key)
                fail(ErrorCode::wrong_scope);
            for (std::size_t i = 0; i < slot.value->record.device_count; ++i)
                if (slot.value->record.devices[i].public_key == key)
                    fail(ErrorCode::wrong_scope);
        }
        DeviceSet candidate;
        candidate.authority = {AuthorityKind::membership, id, key};
        state_->check_roles(candidate);
        operation.check();
        profile.communities.push_back({id, key}); // Reserved during profile construction.
        return id;
    });
}
Result<void> IdentityService::forget_community(ProfileHandle handle, CommunityId id, std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        auto &pins = state_->get(handle).communities;
        const auto found =
            std::find_if(pins.begin(), pins.end(), [&](const auto &pin) { return pin.id == id; });
        if (found == pins.end())
            fail(ErrorCode::untrusted_authority);
        operation.check();
        pins.erase(found); // Does not destroy independently owned membership keys.
    });
}

Result<MembershipHandle> IdentityService::prepare_membership(ProfileHandle handle, CommunityId community,
                                                             std::string_view alias, std::string_view name,
                                                             VerificationTime supplied,
                                                             std::stop_token stop) {
    return state_->run<MembershipHandle>(stop, [&](detail::Operation &operation) {
        auto &profile = state_->get(handle);
        auto labels = make_labels(alias, name);
        if (std::none_of(profile.communities.begin(), profile.communities.end(),
                         [&](const auto &pin) { return pin.id == community; }))
            fail(ErrorCode::untrusted_authority);
        for (const auto &existing : profile.memberships)
            if (existing.value && existing.value->community == community)
                fail(ErrorCode::invalid_state);
        const auto slot = state_->free_slot(profile.memberships);
        state_->keys_available(2);
        state_->bytes_available(0,
                                device_set_header_bytes + crypto::signature_bytes + device_set_entry_bytes);
        const auto now = state_->time(supplied);
        auto member = std::make_unique<detail::Membership>();
        member->community = community;
        member->labels = std::move(labels);
        auto root = state_->generate(operation);
        member->authority.key = std::move(root.key);
        member->authority.record.authority = {AuthorityKind::membership, community, root.public_key};
        member->id = operation.take(
            derive_membership_id(state_->provider, community, root.public_key, operation.token()));
        for (const auto &other_profile : state_->profiles)
            if (other_profile.value)
                for (const auto &existing : other_profile.value->memberships)
                    if (existing.value && existing.value->id == member->id)
                        fail(ErrorCode::crypto_failure);
        auto device = state_->generate(operation);
        if (device.public_key == root.public_key)
            fail(ErrorCode::crypto_failure);
        device.membership_id = operation.take(
            derive_membership_device_id(state_->provider, member->id, device.public_key, operation.token()));
        state_->check_device_id(device, AuthorityKind::membership);
        member->authority.devices[0] = std::move(device);
        state_->initial_record(member->authority, now, operation);
        const auto result = state_->handle<detail::MembershipTag>(slot, profile.memberships[slot].generation,
                                                                  handle.slot_, handle.generation_);
        operation.check();
        profile.memberships[slot].value = std::move(member);
        return result;
    });
}
Result<MembershipSnapshot> IdentityService::membership(ProfileHandle profile, MembershipHandle handle,
                                                       std::stop_token stop) {
    return state_->run<MembershipSnapshot>(stop, [&](detail::Operation &operation) {
        auto result = state_->snapshot(handle, state_->get(profile, handle));
        operation.check();
        return result;
    });
}
Result<std::vector<MembershipSnapshot>> IdentityService::memberships(ProfileHandle handle,
                                                                     std::stop_token stop) {
    return state_->run<std::vector<MembershipSnapshot>>(stop, [&](detail::Operation &operation) {
        const auto &profile = state_->get(handle);
        std::vector<MembershipSnapshot> result;
        result.reserve(profile.memberships.size());
        for (std::size_t i = 0; i < profile.memberships.size(); ++i)
            if (profile.memberships[i].value)
                result.push_back(state_->snapshot(
                    state_->handle<detail::MembershipTag>(i, profile.memberships[i].generation, handle.slot_,
                                                          handle.generation_),
                    *profile.memberships[i].value));
        operation.check();
        return result;
    });
}
Result<std::vector<MembershipSnapshot>> IdentityService::find_memberships(ProfileHandle handle,
                                                                          CommunityId community,
                                                                          std::string_view alias,
                                                                          std::stop_token stop) {
    return state_->run<std::vector<MembershipSnapshot>>(stop, [&](detail::Operation &operation) {
        const auto &profile = state_->get(handle);
        if (!valid_alias(alias))
            fail(ErrorCode::invalid_argument);
        std::vector<MembershipSnapshot> result;
        for (std::size_t i = 0; i < profile.memberships.size(); ++i) {
            const auto &slot = profile.memberships[i];
            if (slot.value && slot.value->community == community && slot.value->labels.alias == alias)
                result.push_back(state_->snapshot(state_->handle<detail::MembershipTag>(
                                                      i, slot.generation, handle.slot_, handle.generation_),
                                                  *slot.value));
        }
        operation.check();
        return result;
    });
}
Result<void> IdentityService::rename_membership(ProfileHandle profile, MembershipHandle handle,
                                                std::string_view alias, std::string_view name,
                                                std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        auto &member = state_->get(profile, handle);
        auto labels = make_labels(alias, name);
        operation.check();
        member.labels = std::move(labels);
    });
}
Result<void> IdentityService::delete_membership(ProfileHandle profile, MembershipHandle handle,
                                                std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        const auto expected = state_->get(profile, handle).authority.record.authority;
        auto &owner = state_->get(profile);
        operation.check();
        // Removing the owned record also removes its point-of-use revocation
        // guard. Invalidate only this profile's self-pin so an older cached
        // record cannot revive a locally revoked device after deletion.
        // These erasures cannot allocate or fail after the publication point.
        for (auto &slot : owner.trusts)
            if (slot.value && slot.value->expected == expected)
                state_->erase(slot);
        state_->erase(owner.memberships[handle.slot_]);
    });
}
Result<DeviceSetFrame> IdentityService::profile_record(ProfileHandle handle, std::stop_token stop) {
    return state_->run<DeviceSetFrame>(stop, [&](detail::Operation &operation) {
        auto result = state_->get(handle).authority.frame;
        operation.check();
        return result;
    });
}
Result<DeviceSetFrame> IdentityService::membership_record(ProfileHandle profile, MembershipHandle handle,
                                                          std::stop_token stop) {
    return state_->run<DeviceSetFrame>(stop, [&](detail::Operation &operation) {
        auto result = state_->get(profile, handle).authority.frame;
        operation.check();
        return result;
    });
}

Result<ProfileDeviceId> IdentityService::add_profile_device(ProfileHandle handle, VerificationTime time,
                                                            std::stop_token stop) {
    return state_->run<ProfileDeviceId>(stop, [&](detail::Operation &operation) {
        auto &profile = state_->get(handle);
        return state_
            ->change(profile.authority, detail::ServiceState::Change::add, 0, profile.id, {}, time, operation)
            .profile_id;
    });
}
Result<MembershipDeviceId> IdentityService::add_membership_device(ProfileHandle profile,
                                                                  MembershipHandle handle,
                                                                  VerificationTime time,
                                                                  std::stop_token stop) {
    return state_->run<MembershipDeviceId>(stop, [&](detail::Operation &operation) {
        auto &member = state_->get(profile, handle);
        return state_
            ->change(member.authority, detail::ServiceState::Change::add, 0, {}, member.id, time, operation)
            .membership_id;
    });
}
Result<ProfileDeviceId> IdentityService::rotate_profile_device(ProfileHandle handle, ProfileDeviceId device,
                                                               VerificationTime time, std::stop_token stop) {
    return state_->run<ProfileDeviceId>(stop, [&](detail::Operation &operation) {
        auto &profile = state_->get(handle);
        std::size_t index = 0;
        while (index < profile.authority.device_count &&
               profile.authority.devices[index].profile_id != device)
            ++index;
        return state_
            ->change(profile.authority, detail::ServiceState::Change::rotate, index, profile.id, {}, time,
                     operation)
            .profile_id;
    });
}
Result<MembershipDeviceId> IdentityService::rotate_membership_device(ProfileHandle profile,
                                                                     MembershipHandle handle,
                                                                     MembershipDeviceId device,
                                                                     VerificationTime time,
                                                                     std::stop_token stop) {
    return state_->run<MembershipDeviceId>(stop, [&](detail::Operation &operation) {
        auto &member = state_->get(profile, handle);
        std::size_t index = 0;
        while (index < member.authority.device_count &&
               member.authority.devices[index].membership_id != device)
            ++index;
        return state_
            ->change(member.authority, detail::ServiceState::Change::rotate, index, {}, member.id, time,
                     operation)
            .membership_id;
    });
}
Result<void> IdentityService::revoke_profile_device(ProfileHandle handle, ProfileDeviceId device,
                                                    VerificationTime time, std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        auto &profile = state_->get(handle);
        std::size_t index = 0;
        while (index < profile.authority.device_count &&
               profile.authority.devices[index].profile_id != device)
            ++index;
        (void)state_->change(profile.authority, detail::ServiceState::Change::revoke, index, profile.id, {},
                             time, operation);
    });
}
Result<void> IdentityService::revoke_membership_device(ProfileHandle profile, MembershipHandle handle,
                                                       MembershipDeviceId device, VerificationTime time,
                                                       std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        auto &member = state_->get(profile, handle);
        std::size_t index = 0;
        while (index < member.authority.device_count &&
               member.authority.devices[index].membership_id != device)
            ++index;
        (void)state_->change(member.authority, detail::ServiceState::Change::revoke, index, {}, member.id,
                             time, operation);
    });
}
Result<void> IdentityService::renew_profile_record(ProfileHandle handle, VerificationTime time,
                                                   std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        auto &profile = state_->get(handle);
        (void)state_->change(profile.authority, detail::ServiceState::Change::renew, 0, profile.id, {}, time,
                             operation);
    });
}
Result<void> IdentityService::renew_membership_record(ProfileHandle profile, MembershipHandle handle,
                                                      VerificationTime time, std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        auto &member = state_->get(profile, handle);
        (void)state_->change(member.authority, detail::ServiceState::Change::renew, 0, {}, member.id, time,
                             operation);
    });
}

Result<TrustHandle> IdentityService::pin_authority(ProfileHandle handle, ExpectedAuthority expected,
                                                   std::stop_token stop) {
    return state_->run<TrustHandle>(stop, [&](detail::Operation &operation) {
        auto &profile = state_->get(handle);
        valid_expected(expected);
        DeviceSet proposed;
        proposed.authority = expected;
        state_->check_roles(proposed, &profile);
        for (std::size_t i = 0; i < profile.trusts.size(); ++i) {
            const auto &slot = profile.trusts[i];
            if (!slot.value)
                continue;
            if (slot.value->expected == expected)
                return state_->handle<detail::TrustTag>(i, slot.generation, handle.slot_, handle.generation_);
            if (slot.value->expected.public_key == expected.public_key)
                fail(ErrorCode::wrong_scope);
        }
        state_->trust_available();
        const auto slot = state_->free_slot(profile.trusts);
        auto trust = std::make_unique<detail::Trust>();
        trust->expected = expected;
        const auto result = state_->handle<detail::TrustTag>(slot, profile.trusts[slot].generation,
                                                             handle.slot_, handle.generation_);
        operation.check();
        profile.trusts[slot].value = std::move(trust);
        return result;
    });
}
Result<TrustSnapshot> IdentityService::trust(ProfileHandle profile, TrustHandle handle,
                                             std::stop_token stop) {
    return state_->run<TrustSnapshot>(stop, [&](detail::Operation &operation) {
        const auto &trust = state_->get(profile, handle);
        const auto status = trust.blocked    ? TrustState::blocked
                            : trust.conflict ? TrustState::conflict
                            : trust.verified ? TrustState::independently_verified
                                             : TrustState::pinned;
        operation.check();
        return TrustSnapshot{handle, trust.expected, status, trust.record.revision, trust.frame.size != 0};
    });
}
Result<void> IdentityService::mark_independently_verified(ProfileHandle profile, TrustHandle handle,
                                                          std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        auto &trust = state_->get(profile, handle);
        if (trust.blocked)
            fail(ErrorCode::blocked);
        if (trust.conflict)
            fail(ErrorCode::conflicting_revision);
        operation.check();
        trust.verified = true;
    });
}
Result<void> IdentityService::set_blocked(ProfileHandle profile, TrustHandle handle, bool blocked,
                                          std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        auto &trust = state_->get(profile, handle);
        if (!blocked && trust.conflict)
            fail(ErrorCode::conflicting_revision);
        operation.check();
        trust.blocked = blocked; // Does not clear a conflict or discard its high-water mark.
    });
}
Result<void> IdentityService::forget_authority(ProfileHandle profile, TrustHandle handle,
                                               std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        (void)state_->get(profile, handle);
        operation.check();
        state_->erase(state_->get(profile).trusts[handle.slot_]);
    });
}
Result<DeviceSet> IdentityService::verify_record(std::span<const std::uint8_t> bytes,
                                                 const ExpectedAuthority &expected, VerificationTime supplied,
                                                 std::stop_token stop) {
    return state_->run<DeviceSet>(stop, [&](detail::Operation &operation) {
        if (bytes.size() > state_->limits.input_bytes)
            fail(ErrorCode::malformed_record);
        const auto now = state_->time(supplied);
        auto record =
            operation.take(verify_device_set(state_->provider, bytes, expected, {now, true},
                                             state_->limits.maximum_lifetime_seconds, operation.token()));
        state_->check_roles(record);
        operation.check();
        return record;
    });
}
Result<void> IdentityService::accept_record(ProfileHandle profile, TrustHandle handle,
                                            std::span<const std::uint8_t> bytes, VerificationTime time,
                                            std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        state_->accept(state_->get(profile), state_->get(profile, handle), bytes, time, false, operation);
    });
}
Result<void> IdentityService::resolve_conflict(ProfileHandle profile, TrustHandle handle,
                                               std::span<const std::uint8_t> bytes, VerificationTime time,
                                               std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        state_->accept(state_->get(profile), state_->get(profile, handle), bytes, time, true, operation);
    });
}
Result<void> IdentityService::recognize_device(ProfileHandle profile, TrustHandle handle,
                                               const crypto::PublicKey &key, VerificationTime supplied,
                                               std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        const auto &trust = state_->get(profile, handle);
        if (trust.blocked)
            fail(ErrorCode::blocked);
        if (trust.conflict)
            fail(ErrorCode::conflicting_revision);
        if (trust.frame.size == 0)
            fail(ErrorCode::unauthorized_device);
        const auto now = state_->time(supplied);
        state_->recognized_by(trust.record, key, now);
        if (const auto *owned = state_->owned_record(state_->get(profile), trust.expected))
            state_->recognized_by(*owned, key, now);
        operation.check();
    });
}
Result<void> IdentityService::revalidate_time(VerificationTime supplied, std::stop_token stop) {
    return state_->run<void>(stop, [&](detail::Operation &operation) {
        if (!supplied.trusted || supplied.seconds > maximum_utc_seconds)
            fail(ErrorCode::time_untrusted);
        operation.check();
        state_->largest_utc = supplied.seconds; // Explicit local recovery; revision/conflict state retained.
        state_->time_requires_revalidation = false;
    });
}
} // namespace ghostwire::identity
