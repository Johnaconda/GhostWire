#include "ghostwire/identity/identity_record.hpp"
#include <algorithm>
#include <string_view>

namespace ghostwire::identity {
namespace {
constexpr char device_set_domain[] = "GhostWire.DeviceSet.v1";
static_assert(sizeof(device_set_domain) == 23);
static_assert(device_set_header_bytes + maximum_devices * device_set_entry_bytes + crypto::signature_bytes ==
              maximum_device_set_bytes);

template <typename Id, std::size_t N>
Result<Id> derive(crypto::CryptoProvider &provider, const char (&domain)[N],
                  std::span<const std::uint8_t> scope, const crypto::PublicKey &key, std::stop_token stop) {
    static_assert(N + 64 <= 128);
    std::array<std::uint8_t, 128> input{};
    for (std::size_t i = 0; i < N; ++i)
        input[i] = static_cast<std::uint8_t>(domain[i]); // Includes exactly one NUL.
    std::copy(scope.begin(), scope.end(), input.begin() + N);
    std::copy(key.begin(), key.end(), input.begin() + static_cast<std::ptrdiff_t>(N + scope.size()));
    auto digest = provider.sha256(std::span(input).first(N + scope.size() + key.size()), stop);
    if (!digest)
        return from_crypto_error(digest.error());
    return Id{digest.value()};
}

bool valid_authority(const ExpectedAuthority &authority) noexcept {
    if (authority.kind == AuthorityKind::profile)
        return authority.scope == CommunityId{};
    return authority.kind == AuthorityKind::membership;
}

Result<void> validate(const DeviceSet &set, std::uint64_t maximum_lifetime) noexcept {
    if (maximum_lifetime == 0 || maximum_lifetime > maximum_device_set_lifetime)
        return Error{ErrorCode::invalid_argument};
    if (!valid_authority(set.authority) || set.revision == 0 || set.device_count > maximum_devices ||
        set.valid_from >= set.valid_until || set.valid_until > maximum_utc_seconds ||
        set.valid_until - set.valid_from > maximum_lifetime)
        return Error{ErrorCode::malformed_record};
    for (std::size_t i = 0; i < set.device_count; ++i) {
        const auto &device = set.devices[i];
        if (device.valid_from >= device.valid_until || device.valid_from < set.valid_from ||
            device.valid_until > set.valid_until || device.public_key == set.authority.public_key ||
            (i > 0 && !(set.devices[i - 1].public_key < device.public_key)))
            return Error{ErrorCode::malformed_record};
    }
    return {};
}

void write_integer(DeviceSetFrame &frame, std::size_t &offset, std::uint64_t value,
                   std::size_t width) noexcept {
    for (std::size_t i = 0; i < width; ++i) {
        frame.bytes[offset + width - 1 - i] = static_cast<std::uint8_t>(value & 0xffU);
        value >>= 8U;
    }
    offset += width;
}
void write_bytes(DeviceSetFrame &frame, std::size_t &offset, std::span<const std::uint8_t> bytes) noexcept {
    std::copy(bytes.begin(), bytes.end(), frame.bytes.begin() + static_cast<std::ptrdiff_t>(offset));
    offset += bytes.size();
}
std::uint64_t read_integer(std::span<const std::uint8_t> frame, std::size_t &offset,
                           std::size_t width) noexcept {
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < width; ++i)
        value = (value << 8U) | frame[offset++];
    return value;
}
void read_bytes(std::span<const std::uint8_t> frame, std::size_t &offset,
                std::span<std::uint8_t> bytes) noexcept {
    std::copy_n(frame.begin() + static_cast<std::ptrdiff_t>(offset), bytes.size(), bytes.begin());
    offset += bytes.size();
}
} // namespace

Result<ProfileIdentityId> derive_profile_id(crypto::CryptoProvider &provider, const crypto::PublicKey &key,
                                            std::stop_token stop) {
    return derive<ProfileIdentityId>(provider, "GhostWire.ProfileRoot.v1", {}, key, stop);
}
Result<CommunityId> derive_community_id(crypto::CryptoProvider &provider, const crypto::PublicKey &key,
                                        std::stop_token stop) {
    return derive<CommunityId>(provider, "GhostWire.CommunityRoot.v1", {}, key, stop);
}
Result<MembershipId> derive_membership_id(crypto::CryptoProvider &provider, const CommunityId &scope,
                                          const crypto::PublicKey &key, std::stop_token stop) {
    return derive<MembershipId>(provider, "GhostWire.Membership.v1", scope.bytes, key, stop);
}
Result<ProfileDeviceId> derive_profile_device_id(crypto::CryptoProvider &provider,
                                                 const ProfileIdentityId &scope, const crypto::PublicKey &key,
                                                 std::stop_token stop) {
    return derive<ProfileDeviceId>(provider, "GhostWire.ProfileDevice.v1", scope.bytes, key, stop);
}
Result<MembershipDeviceId> derive_membership_device_id(crypto::CryptoProvider &provider,
                                                       const MembershipId &scope,
                                                       const crypto::PublicKey &key, std::stop_token stop) {
    return derive<MembershipDeviceId>(provider, "GhostWire.MembershipDevice.v1", scope.bytes, key, stop);
}

Result<DeviceSetFrame> sign_device_set(crypto::CryptoProvider &provider, const crypto::SigningKey &key,
                                       const DeviceSet &set, std::stop_token stop) {
    if (stop.stop_requested())
        return Error{ErrorCode::cancelled};
    auto validated = validate(set, maximum_device_set_lifetime);
    if (!validated)
        return validated.error();
    auto public_key = provider.public_key(key, stop);
    if (!public_key)
        return from_crypto_error(public_key.error());
    if (public_key.value() != set.authority.public_key)
        return Error{ErrorCode::untrusted_authority};
    DeviceSetFrame frame;
    std::size_t offset = 0;
    for (const auto byte : device_set_domain)
        frame.bytes[offset++] = static_cast<std::uint8_t>(byte);
    write_integer(frame, offset, 1, 2);
    write_integer(frame, offset, 1, 2);
    write_integer(frame, offset, static_cast<std::uint8_t>(set.authority.kind), 1);
    write_bytes(frame, offset, set.authority.scope.bytes);
    write_bytes(frame, offset, set.authority.public_key);
    write_integer(frame, offset, set.revision, 8);
    write_integer(frame, offset, set.valid_from, 8);
    write_integer(frame, offset, set.valid_until, 8);
    write_integer(frame, offset, set.device_count, 2);
    for (std::size_t i = 0; i < set.device_count; ++i) {
        write_bytes(frame, offset, set.devices[i].public_key);
        write_integer(frame, offset, set.devices[i].valid_from, 8);
        write_integer(frame, offset, set.devices[i].valid_until, 8);
    }
    auto signature = provider.sign_public(key, std::span(frame.bytes).first(offset), stop);
    if (!signature)
        return from_crypto_error(signature.error());
    write_bytes(frame, offset, signature.value());
    frame.size = offset;
    if (stop.stop_requested())
        return Error{ErrorCode::cancelled};
    return frame;
}

Result<DeviceSet> verify_device_set(crypto::CryptoProvider &provider, std::span<const std::uint8_t> frame,
                                    const ExpectedAuthority &expected, VerificationTime now,
                                    std::uint64_t maximum_lifetime, std::stop_token stop) {
    if (stop.stop_requested())
        return Error{ErrorCode::cancelled};
    if (!now.trusted || now.seconds > maximum_utc_seconds)
        return Error{ErrorCode::time_untrusted};
    if (maximum_lifetime == 0 || maximum_lifetime > maximum_device_set_lifetime)
        return Error{ErrorCode::invalid_argument};
    if (frame.size() < device_set_header_bytes + crypto::signature_bytes ||
        frame.size() > maximum_device_set_bytes)
        return Error{ErrorCode::malformed_record};
    std::size_t offset = 0;
    for (const auto byte : device_set_domain)
        if (frame[offset++] != static_cast<std::uint8_t>(byte))
            return Error{ErrorCode::malformed_record};
    if (read_integer(frame, offset, 2) != 1)
        return Error{ErrorCode::unsupported_version};
    if (read_integer(frame, offset, 2) != 1)
        return Error{ErrorCode::unsupported_suite};
    DeviceSet set;
    set.authority.kind = static_cast<AuthorityKind>(read_integer(frame, offset, 1));
    read_bytes(frame, offset, set.authority.scope.bytes);
    read_bytes(frame, offset, set.authority.public_key);
    set.revision = read_integer(frame, offset, 8);
    set.valid_from = read_integer(frame, offset, 8);
    set.valid_until = read_integer(frame, offset, 8);
    set.device_count = static_cast<std::size_t>(read_integer(frame, offset, 2));
    if (set.device_count > maximum_devices || frame.size() != device_set_header_bytes +
                                                                  set.device_count * device_set_entry_bytes +
                                                                  crypto::signature_bytes)
        return Error{ErrorCode::malformed_record};
    for (std::size_t i = 0; i < set.device_count; ++i) {
        read_bytes(frame, offset, set.devices[i].public_key);
        set.devices[i].valid_from = read_integer(frame, offset, 8);
        set.devices[i].valid_until = read_integer(frame, offset, 8);
    }
    auto validated = validate(set, maximum_lifetime);
    if (!validated)
        return validated.error();
    if (!valid_authority(expected) || set.authority.kind != expected.kind ||
        set.authority.scope != expected.scope)
        return Error{ErrorCode::wrong_scope};
    if (set.authority.public_key != expected.public_key)
        return Error{ErrorCode::untrusted_authority};
    auto signature =
        provider.verify_public(expected.public_key, frame.first(offset), frame.subspan(offset), stop);
    if (!signature)
        return from_crypto_error(signature.error());
    if (stop.stop_requested())
        return Error{ErrorCode::cancelled};
    if (now.seconds < set.valid_from)
        return Error{ErrorCode::not_yet_valid};
    if (now.seconds >= set.valid_until)
        return Error{ErrorCode::expired};
    return set;
}
} // namespace ghostwire::identity
