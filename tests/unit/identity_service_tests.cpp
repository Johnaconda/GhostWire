#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ghostwire/identity/identity_service.hpp"
#include <algorithm>
#include <atomic>
#include <doctest/doctest.h>
#include <limits>
#include <type_traits>

namespace gi = ghostwire::identity;
namespace gc = ghostwire::crypto;
namespace {
template <typename T, typename E> T take(ghostwire::core::Result<T, E> result) {
    REQUIRE(result);
    return std::move(result).value();
}
template <typename T> void rejected(const gi::Result<T> &result, gi::ErrorCode expected) {
    REQUIRE_FALSE(result);
    CHECK(result.error().code == expected);
    CHECK_FALSE(gi::describe(result.error().code).empty());
}
class ManualClock final : public ghostwire::core::Clock {
  public:
    TimePoint now() const noexcept override { return TimePoint(std::chrono::milliseconds(ticks.load())); }
    std::atomic<std::chrono::milliseconds::rep> ticks{0};
};
constexpr gi::VerificationTime current{1000, true};

// These are test-owned external authority keys, never extracted from a profile.
// DeviceSet encoding/signature primitives have independent fixed-vector tests.
struct ExternalAuthority {
    gc::CryptoProvider &crypto;
    gc::SigningKey key;
    gc::SigningKey device;
    gi::ExpectedAuthority expected;
    gc::PublicKey device_public;
    explicit ExternalAuthority(gc::CryptoProvider &provider,
                               gi::AuthorityKind kind = gi::AuthorityKind::profile,
                               gi::CommunityId scope = {})
        : crypto(provider), key(take(provider.generate_signing_key())),
          device(take(provider.generate_signing_key())),
          expected{kind, scope, take(provider.public_key(key))},
          device_public(take(provider.public_key(device))) {}

    gi::DeviceSetFrame record(std::uint64_t revision = 1, std::uint64_t from = 900,
                              std::uint64_t until = 2000, bool include_device = true) {
        gi::DeviceSet value;
        value.authority = expected;
        value.revision = revision;
        value.valid_from = from;
        value.valid_until = until;
        if (include_device) {
            value.device_count = 1;
            value.devices[0] = {device_public, from, until};
        }
        return take(gi::sign_device_set(crypto, key, value));
    }
};
bool contains_public_key(std::span<const std::uint8_t> bytes, const gc::PublicKey &key) {
    return std::search(bytes.begin(), bytes.end(), key.begin(), key.end()) != bytes.end();
}
struct Harness {
    ManualClock clock;
    std::unique_ptr<gc::CryptoProvider> crypto = take(gc::CryptoProvider::create());
    std::unique_ptr<gi::IdentityService> service;
    explicit Harness(gi::ServiceLimits limits = {})
        : service(take(gi::IdentityService::create(*crypto, clock, limits))) {}
    gi::ProfileHandle profile(std::string_view alias = "00123") {
        return take(service->create_profile(alias, "J\xc3\xb6rg \xe9\x9b\xaa", current));
    }
    gc::PublicKey community_key() {
        auto key = take(crypto->generate_signing_key());
        return take(crypto->public_key(key));
    }
};
} // namespace

static_assert(!std::is_copy_constructible_v<gi::IdentityService>);
static_assert(!std::is_move_constructible_v<gi::IdentityService>);
static_assert(!std::is_convertible_v<gi::ProfileIdentityId, gi::ProfileHandle>);
static_assert(!std::is_convertible_v<gi::MembershipId, gi::MembershipHandle>);
static_assert(!std::is_convertible_v<gi::ProfileHandle, gi::MembershipHandle>);
static_assert(!std::is_convertible_v<gi::ProfileDeviceId, gi::MembershipDeviceId>);
static_assert(!std::is_convertible_v<gi::DeviceSet, gi::ProfileHandle>);

TEST_CASE("identity ID01 isolated profiles retain independent keys and immutable public snapshots") {
    Harness h;
    auto first = h.profile(), second = h.profile();
    auto a = take(h.service->profile(first)), b = take(h.service->profile(second));
    CHECK(first != second);
    CHECK(a.id != b.id);
    CHECK(a.public_key != b.public_key);
    CHECK(a.labels == b.labels);
    REQUIRE(a.devices.size() == 1);
    REQUIRE(b.devices.size() == 1);
    CHECK(a.devices[0].public_key != b.devices[0].public_key);
    CHECK(a.public_key != a.devices[0].public_key);
    a.labels.alias = "999";
    a.devices.clear();
    auto unchanged = take(h.service->profile(first));
    CHECK(unchanged.labels.alias == "00123");
    CHECK(unchanged.devices.size() == 1);
    CHECK(h.service->diagnostic().profiles == 2);
    CHECK(h.service->diagnostic().private_keys == 4);
    CHECK_FALSE(h.service->diagnostic().persistence_available);
}

TEST_CASE("identity ID02 same alias in one community never merges separate profile memberships") {
    Harness h;
    auto a = h.profile(), b = h.profile();
    const auto community_key = h.community_key();
    const auto ca = take(h.service->pin_community(a, community_key));
    const auto cb = take(h.service->pin_community(b, community_key));
    CHECK(ca == cb);
    auto ma = take(h.service->prepare_membership(a, ca, "00017", "same name", current));
    auto mb = take(h.service->prepare_membership(b, cb, "00017", "same name", current));
    auto sa = take(h.service->membership(a, ma)), sb = take(h.service->membership(b, mb));
    CHECK(sa.id != sb.id);
    CHECK(sa.public_key != sb.public_key);
    REQUIRE(sa.devices.size() == 1);
    REQUIRE(sb.devices.size() == 1);
    CHECK(sa.devices[0].id != sb.devices[0].id);
    CHECK(sa.devices[0].public_key != sb.devices[0].public_key);
    auto aa = take(h.service->find_memberships(a, ca, "00017"));
    auto ab = take(h.service->find_memberships(b, cb, "00017"));
    REQUIRE(aa.size() == 1);
    REQUIRE(ab.size() == 1);
    CHECK(aa[0].id == sa.id);
    CHECK(ab[0].id == sb.id);
    CHECK(take(h.service->find_memberships(a, ca, "17")).empty());
    rejected(h.service->membership(a, mb), gi::ErrorCode::invalid_handle);
    rejected(h.service->membership(b, ma), gi::ErrorCode::invalid_handle);
    rejected(h.service->prepare_membership(a, ca, "00017", "duplicate", current),
             gi::ErrorCode::invalid_state);
}

TEST_CASE("identity ID03 membership credentials are scoped and disclose no profile linkage") {
    Harness h;
    auto profile = h.profile();
    const auto local = take(h.service->profile(profile));
    const auto ca = take(h.service->pin_community(profile, h.community_key()));
    const auto cb = take(h.service->pin_community(profile, h.community_key()));
    CHECK(ca != cb);
    auto ma = take(h.service->prepare_membership(profile, ca, "8", "A", current));
    auto mb = take(h.service->prepare_membership(profile, cb, "8", "B", current));
    const auto a = take(h.service->membership(profile, ma));
    const auto b = take(h.service->membership(profile, mb));
    CHECK(a.id != b.id);
    CHECK(a.public_key != b.public_key);
    CHECK(a.devices[0].public_key != b.devices[0].public_key);
    const auto frame = take(h.service->membership_record(profile, ma));
    CHECK_FALSE(contains_public_key(frame.view(), local.public_key));
    CHECK_FALSE(contains_public_key(frame.view(), local.devices[0].public_key));
    gi::ExpectedAuthority expected{gi::AuthorityKind::membership, ca, a.public_key};
    CHECK(h.service->verify_record(frame.view(), expected, current));
    expected.scope = cb;
    rejected(h.service->verify_record(frame.view(), expected, current), gi::ErrorCode::wrong_scope);
    gi::CommunityId unpinned;
    rejected(h.service->prepare_membership(profile, unpinned, "9", "unknown", current),
             gi::ErrorCode::untrusted_authority);
}

TEST_CASE("identity ID04 copied public records provide neither private ownership nor implicit trust") {
    Harness alice, bob;
    const auto a = alice.profile(), b = bob.profile();
    const auto snapshot = take(alice.service->profile(a));
    const auto public_record = take(alice.service->profile_record(a));
    const gi::ExpectedAuthority expected{gi::AuthorityKind::profile, {}, snapshot.public_key};
    CHECK(bob.service->verify_record(public_record.view(), expected, current));
    CHECK(bob.service->diagnostic().trust_contexts == 0);
    CHECK(bob.service->diagnostic().private_keys == 2);
    rejected(bob.service->profile(a), gi::ErrorCode::invalid_handle);
    rejected(bob.service->delete_profile(a), gi::ErrorCode::invalid_handle);
    rejected(bob.service->rotate_profile_device(a, snapshot.devices[0].id, current),
             gi::ErrorCode::invalid_handle);
    rejected(bob.service->accept_record(b, {}, public_record.view(), current), gi::ErrorCode::invalid_handle);
    CHECK(take(alice.service->profile(a)).id == snapshot.id);
}

TEST_CASE("identity ID05 deleted reused and cross service handles fail without harming survivors") {
    Harness h, other;
    const auto old = h.profile(), survivor = h.profile();
    const auto original = take(h.service->profile(old));
    REQUIRE(h.service->delete_profile(old));
    const auto replacement = h.profile();
    CHECK(replacement != old);
    CHECK(take(h.service->profile(replacement)).id != original.id);
    rejected(h.service->profile(old), gi::ErrorCode::invalid_handle);
    rejected(h.service->rename_profile(old, "1", "stale"), gi::ErrorCode::invalid_handle);
    rejected(other.service->profile(survivor), gi::ErrorCode::invalid_handle);
    CHECK(h.service->profile(survivor));
    rejected(h.service->profile({}), gi::ErrorCode::invalid_handle);
}

TEST_CASE("identity ID06 renames preserve principals revocation and pinned trust") {
    Harness h;
    const auto p = h.profile();
    const auto c = take(h.service->pin_community(p, h.community_key()));
    const auto m = take(h.service->prepare_membership(p, c, "0001", "before", current));
    const auto before_p = take(h.service->profile(p));
    const auto before_m = take(h.service->membership(p, m));
    const auto record_p = take(h.service->profile_record(p));
    ExternalAuthority authority(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    REQUIRE(h.service->accept_record(p, trust, authority.record(3).view(), current));
    REQUIRE(h.service->revoke_membership_device(p, m, before_m.devices[0].id, current));
    REQUIRE(h.service->rename_profile(p, "0002", "\xc3\x96 new"));
    REQUIRE(h.service->rename_membership(p, m, "0003", "\xe9\x9b\xaa renamed"));
    CHECK(take(h.service->profile(p)).id == before_p.id);
    CHECK(take(h.service->profile_record(p)) == record_p);
    const auto after_m = take(h.service->membership(p, m));
    CHECK(after_m.id == before_m.id);
    CHECK(after_m.devices.empty());
    CHECK(after_m.revision == before_m.revision + 1);
    CHECK(after_m.labels.alias == "0003");
    CHECK(take(h.service->trust(p, trust)).highest_revision == 3);
    CHECK(h.service->recognize_device(p, trust, authority.device_public, current));
}

TEST_CASE("identity ID07 pins and independent verification are explicit per profile") {
    Harness h;
    const auto a = h.profile(), b = h.profile();
    ExternalAuthority authority(*h.crypto);
    const auto frame = authority.record();
    CHECK(h.service->verify_record(frame.view(), authority.expected, current));
    CHECK(h.service->diagnostic().trust_contexts == 0);
    const auto pinned = take(h.service->pin_authority(a, authority.expected));
    CHECK(take(h.service->trust(a, pinned)).state == gi::TrustState::pinned);
    rejected(h.service->recognize_device(a, pinned, authority.device_public, current),
             gi::ErrorCode::unauthorized_device);
    REQUIRE(h.service->accept_record(a, pinned, frame.view(), current));
    CHECK(h.service->recognize_device(a, pinned, authority.device_public, current));
    rejected(h.service->recognize_device(b, pinned, authority.device_public, current),
             gi::ErrorCode::invalid_handle);
    REQUIRE(h.service->mark_independently_verified(a, pinned));
    CHECK(take(h.service->trust(a, pinned)).state == gi::TrustState::independently_verified);
    REQUIRE(h.service->set_blocked(a, pinned, true));
    rejected(h.service->recognize_device(a, pinned, authority.device_public, current),
             gi::ErrorCode::blocked);
    rejected(h.service->accept_record(a, pinned, frame.view(), current), gi::ErrorCode::blocked);
    REQUIRE(h.service->set_blocked(a, pinned, false));
    CHECK(h.service->recognize_device(a, pinned, authority.device_public, current));
}

TEST_CASE("identity ID08 substituted authority and scope never update the selected trust context") {
    Harness h;
    const auto p = h.profile();
    ExternalAuthority authority(*h.crypto), impostor(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    REQUIRE(h.service->accept_record(p, trust, authority.record().view(), current));
    auto bad = h.service->accept_record(p, trust, impostor.record(2).view(), current);
    REQUIRE_FALSE(bad);
    CHECK(take(h.service->trust(p, trust)).highest_revision == 1);
    auto altered = authority.record(2);
    altered.bytes[altered.size - 1] ^= 1;
    rejected(h.service->accept_record(p, trust, altered.view(), current), gi::ErrorCode::invalid_signature);
    CHECK(take(h.service->trust(p, trust)).highest_revision == 1);
    CHECK(h.service->recognize_device(p, trust, authority.device_public, current));
    rejected(h.service->recognize_device(p, trust, impostor.device_public, current),
             gi::ErrorCode::unauthorized_device);
}

TEST_CASE("identity ID12 exact expiration is rechecked during every recognition") {
    Harness h;
    const auto p = h.profile();
    ExternalAuthority authority(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    const auto frame = authority.record(1, 1000, 1010);
    REQUIRE(h.service->accept_record(p, trust, frame.view(), current));
    CHECK(h.service->recognize_device(p, trust, authority.device_public, {1009, true}));
    rejected(h.service->recognize_device(p, trust, authority.device_public, {1010, true}),
             gi::ErrorCode::expired);
    CHECK(take(h.service->trust(p, trust)).highest_revision == 1);
    CHECK_FALSE(h.service->recognize_device(p, trust, authority.device_public, {1009, true}));
}

TEST_CASE("identity ID13 trusted UTC rollback policy is distinct from monotonic deadlines") {
    Harness h;
    const auto p = h.profile();
    ExternalAuthority authority(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    const auto frame = authority.record(7);
    REQUIRE(h.service->accept_record(p, trust, frame.view(), current));
    CHECK(h.service->recognize_device(p, trust, authority.device_public, {1005, true}));
    CHECK(h.service->recognize_device(p, trust, authority.device_public, {1000, true}));
    rejected(h.service->recognize_device(p, trust, authority.device_public, {999, true}),
             gi::ErrorCode::time_untrusted);
    rejected(h.service->recognize_device(p, trust, authority.device_public, {1006, true}),
             gi::ErrorCode::time_untrusted);
    rejected(h.service->recognize_device(p, trust, authority.device_public, {}),
             gi::ErrorCode::time_untrusted);
    rejected(h.service->revalidate_time({std::numeric_limits<std::uint64_t>::max(), true}),
             gi::ErrorCode::time_untrusted);
    REQUIRE(h.service->revalidate_time({999, true}));
    CHECK(h.service->recognize_device(p, trust, authority.device_public, {999, true}));
    rejected(h.service->accept_record(p, trust, authority.record(6).view(), {999, true}),
             gi::ErrorCode::stale_revision);
    CHECK(take(h.service->trust(p, trust)).highest_revision == 7);
    rejected(h.service->recognize_device(p, trust, authority.device_public, {2000, true}),
             gi::ErrorCode::expired);
}

TEST_CASE("identity ID14 complete revision gaps idempotence and stale replay preserve high water") {
    Harness h;
    const auto p = h.profile();
    ExternalAuthority authority(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    const auto first = authority.record(4);
    REQUIRE(h.service->accept_record(p, trust, first.view(), current));
    const auto bytes = h.service->diagnostic().public_record_bytes;
    REQUIRE(h.service->accept_record(p, trust, first.view(), current));
    CHECK(h.service->diagnostic().public_record_bytes == bytes);
    rejected(h.service->accept_record(p, trust, authority.record(3).view(), current),
             gi::ErrorCode::stale_revision);
    REQUIRE(h.service->accept_record(p, trust, authority.record(6).view(), current));
    CHECK(take(h.service->trust(p, trust)).highest_revision == 6);
    CHECK(h.service->diagnostic().public_record_bytes == bytes);
}

TEST_CASE("identity ID15 equivocation blocks recognition until explicit conflict resolution") {
    Harness h;
    const auto p = h.profile();
    ExternalAuthority authority(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    REQUIRE(h.service->accept_record(p, trust, authority.record(5).view(), current));
    const auto conflict = authority.record(5, 900, 2000, false);
    rejected(h.service->accept_record(p, trust, conflict.view(), current),
             gi::ErrorCode::conflicting_revision);
    CHECK(take(h.service->trust(p, trust)).state == gi::TrustState::conflict);
    rejected(h.service->recognize_device(p, trust, authority.device_public, current),
             gi::ErrorCode::conflicting_revision);
    rejected(h.service->accept_record(p, trust, authority.record(7).view(), current),
             gi::ErrorCode::conflicting_revision);
    CHECK_FALSE(h.service->mark_independently_verified(p, trust));
    rejected(h.service->set_blocked(p, trust, false), gi::ErrorCode::conflicting_revision);
    CHECK(take(h.service->trust(p, trust)).state == gi::TrustState::conflict);
    rejected(h.service->recognize_device(p, trust, authority.device_public, current),
             gi::ErrorCode::conflicting_revision);
    rejected(h.service->resolve_conflict(p, trust, authority.record(4).view(), current),
             gi::ErrorCode::stale_revision);
    REQUIRE(h.service->resolve_conflict(p, trust, conflict.view(), current));
    CHECK(take(h.service->trust(p, trust)).state != gi::TrustState::conflict);
    CHECK(take(h.service->trust(p, trust)).highest_revision == 5);
    rejected(h.service->recognize_device(p, trust, authority.device_public, current),
             gi::ErrorCode::unauthorized_device);
}

TEST_CASE("identity blocked conflict can be explicitly resolved without silently unblocking authority") {
    Harness h;
    const auto p = h.profile();
    ExternalAuthority authority(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    REQUIRE(h.service->accept_record(p, trust, authority.record(5).view(), current));
    const auto conflict = authority.record(5, 900, 2000, false);
    rejected(h.service->accept_record(p, trust, conflict.view(), current),
             gi::ErrorCode::conflicting_revision);
    REQUIRE(h.service->set_blocked(p, trust, true));
    REQUIRE(h.service->resolve_conflict(p, trust, conflict.view(), current));
    CHECK(take(h.service->trust(p, trust)).state == gi::TrustState::blocked);
    rejected(h.service->recognize_device(p, trust, authority.device_public, current), gi::ErrorCode::blocked);
    REQUIRE(h.service->set_blocked(p, trust, false));
    rejected(h.service->recognize_device(p, trust, authority.device_public, current),
             gi::ErrorCode::unauthorized_device);
}

TEST_CASE("identity ID16 scoped rotation and revocation use the current record not old snapshots") {
    Harness owner, observer;
    const auto p = owner.profile(), viewer = observer.profile();
    const auto c = take(owner.service->pin_community(p, owner.community_key()));
    const auto m = take(owner.service->prepare_membership(p, c, "22", "member", current));
    const auto profile_before = take(owner.service->profile(p));
    const auto member_before = take(owner.service->membership(p, m));
    const auto root_pin = take(
        observer.service->pin_authority(viewer, {gi::AuthorityKind::profile, {}, profile_before.public_key}));
    const auto member_pin = take(observer.service->pin_authority(
        viewer, {gi::AuthorityKind::membership, c, member_before.public_key}));
    REQUIRE(observer.service->accept_record(viewer, root_pin, take(owner.service->profile_record(p)).view(),
                                            current));
    REQUIRE(observer.service->accept_record(viewer, member_pin,
                                            take(owner.service->membership_record(p, m)).view(), current));
    const auto old_record = take(owner.service->profile_record(p));
    const auto old_device = profile_before.devices[0];
    const auto rotated = take(owner.service->rotate_profile_device(p, old_device.id, current));
    CHECK(rotated != old_device.id);
    const auto profile_after = take(owner.service->profile(p));
    REQUIRE(observer.service->accept_record(viewer, root_pin, take(owner.service->profile_record(p)).view(),
                                            current));
    rejected(observer.service->recognize_device(viewer, root_pin, old_device.public_key, current),
             gi::ErrorCode::unauthorized_device);
    rejected(observer.service->accept_record(viewer, root_pin, old_record.view(), current),
             gi::ErrorCode::stale_revision);
    CHECK(observer.service->recognize_device(viewer, root_pin, profile_after.devices[0].public_key, current));
    CHECK(
        observer.service->recognize_device(viewer, member_pin, member_before.devices[0].public_key, current));
    CHECK(take(owner.service->membership(p, m)).revision == member_before.revision);
    REQUIRE(owner.service->revoke_membership_device(p, m, member_before.devices[0].id, current));
    REQUIRE(observer.service->accept_record(viewer, member_pin,
                                            take(owner.service->membership_record(p, m)).view(), current));
    rejected(
        observer.service->recognize_device(viewer, member_pin, member_before.devices[0].public_key, current),
        gi::ErrorCode::unauthorized_device);
    CHECK(observer.service->recognize_device(viewer, root_pin, profile_after.devices[0].public_key, current));
    CHECK(take(owner.service->membership(p, m)).devices.empty());
    CHECK(owner.service->diagnostic().private_keys == 3);
}

TEST_CASE("identity ID17 wrong profile cannot mutate another membership or pin") {
    Harness h;
    const auto a = h.profile(), b = h.profile();
    const auto c = take(h.service->pin_community(a, h.community_key()));
    const auto m = take(h.service->prepare_membership(a, c, "1", "member", current));
    const auto before = take(h.service->membership(a, m));
    ExternalAuthority authority(*h.crypto);
    const auto trust = take(h.service->pin_authority(a, authority.expected));
    rejected(h.service->add_membership_device(b, m, current), gi::ErrorCode::invalid_handle);
    rejected(h.service->revoke_membership_device(b, m, before.devices[0].id, current),
             gi::ErrorCode::invalid_handle);
    rejected(h.service->delete_membership(b, m), gi::ErrorCode::invalid_handle);
    rejected(h.service->mark_independently_verified(b, trust), gi::ErrorCode::invalid_handle);
    rejected(h.service->set_blocked(b, trust, true), gi::ErrorCode::invalid_handle);
    rejected(h.service->forget_authority(b, trust), gi::ErrorCode::invalid_handle);
    CHECK(take(h.service->membership(a, m)).revision == before.revision);
}

TEST_CASE(
    "identity self pins immediately honor locally owned device revocation without cached record refresh") {
    Harness h;
    const auto p = h.profile(), observer = h.profile();
    const auto local = take(h.service->profile(p));
    const gi::ExpectedAuthority expected{gi::AuthorityKind::profile, {}, local.public_key};
    const auto own_trust = take(h.service->pin_authority(p, expected));
    const auto observer_trust = take(h.service->pin_authority(observer, expected));
    const auto old_record = take(h.service->profile_record(p));
    REQUIRE(h.service->accept_record(p, own_trust, old_record.view(), current));
    REQUIRE(h.service->accept_record(observer, observer_trust, old_record.view(), current));
    REQUIRE(h.service->revoke_profile_device(p, local.devices[0].id, current));
    rejected(h.service->recognize_device(p, own_trust, local.devices[0].public_key, current),
             gi::ErrorCode::unauthorized_device);
    rejected(h.service->accept_record(p, own_trust, old_record.view(), current),
             gi::ErrorCode::stale_revision);
    // An isolated observer profile changes only after explicit receipt/acceptance.
    CHECK(h.service->recognize_device(observer, observer_trust, local.devices[0].public_key, current));
    REQUIRE(h.service->accept_record(observer, observer_trust, take(h.service->profile_record(p)).view(),
                                     current));
    rejected(h.service->recognize_device(observer, observer_trust, local.devices[0].public_key, current),
             gi::ErrorCode::unauthorized_device);
    const auto c = take(h.service->pin_community(p, h.community_key()));
    const auto m = take(h.service->prepare_membership(p, c, "7", "member", current));
    const auto member = take(h.service->membership(p, m));
    const auto member_trust =
        take(h.service->pin_authority(p, {gi::AuthorityKind::membership, c, member.public_key}));
    REQUIRE(
        h.service->accept_record(p, member_trust, take(h.service->membership_record(p, m)).view(), current));
    REQUIRE(h.service->rotate_membership_device(p, m, member.devices[0].id, current));
    rejected(h.service->recognize_device(p, member_trust, member.devices[0].public_key, current),
             gi::ErrorCode::unauthorized_device);
}

TEST_CASE("identity ID18 deleting and recreating principals inherits no private identity or trust") {
    Harness h;
    const auto p = h.profile();
    const auto old = take(h.service->profile(p));
    ExternalAuthority authority(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    REQUIRE(h.service->delete_profile(p));
    const auto replacement = h.profile();
    CHECK(take(h.service->profile(replacement)).id != old.id);
    CHECK(h.service->diagnostic().trust_contexts == 0);
    rejected(h.service->trust(replacement, trust), gi::ErrorCode::invalid_handle);
    CHECK_FALSE(h.service->diagnostic().persistence_available);
}

TEST_CASE("identity membership deletion invalidates self trust without reviving a revoked device") {
    Harness h;
    const auto owner = h.profile(), observer = h.profile();
    const auto community = take(h.service->pin_community(owner, h.community_key()));
    const auto membership =
        take(h.service->prepare_membership(owner, community, "7", "owned member", current));
    const auto member = take(h.service->membership(owner, membership));
    const gi::ExpectedAuthority expected{gi::AuthorityKind::membership, community, member.public_key};
    const auto self_trust = take(h.service->pin_authority(owner, expected));
    const auto observer_trust = take(h.service->pin_authority(observer, expected));
    const auto old_record = take(h.service->membership_record(owner, membership));
    REQUIRE(h.service->accept_record(owner, self_trust, old_record.view(), current));
    REQUIRE(h.service->accept_record(observer, observer_trust, old_record.view(), current));
    ExternalAuthority unrelated(*h.crypto);
    const auto unrelated_trust = take(h.service->pin_authority(owner, unrelated.expected));
    REQUIRE(h.service->accept_record(owner, unrelated_trust, unrelated.record().view(), current));
    REQUIRE(h.service->revoke_membership_device(owner, membership, member.devices[0].id, current));
    rejected(h.service->recognize_device(owner, self_trust, member.devices[0].public_key, current),
             gi::ErrorCode::unauthorized_device);
    const auto revoked_record = take(h.service->membership_record(owner, membership));
    const auto before = h.service->diagnostic();

    std::stop_source cancelled;
    cancelled.request_stop();
    rejected(h.service->delete_membership(owner, membership, cancelled.get_token()),
             gi::ErrorCode::cancelled);
    CHECK(h.service->membership(owner, membership));
    CHECK(h.service->trust(owner, self_trust));
    CHECK(h.service->diagnostic().public_record_bytes == before.public_record_bytes);

    REQUIRE(h.service->delete_membership(owner, membership));
    rejected(h.service->membership(owner, membership), gi::ErrorCode::invalid_handle);
    rejected(h.service->recognize_device(owner, self_trust, member.devices[0].public_key, current),
             gi::ErrorCode::invalid_handle);
    rejected(h.service->trust(owner, self_trust), gi::ErrorCode::invalid_handle);
    rejected(h.service->accept_record(owner, self_trust, old_record.view(), current),
             gi::ErrorCode::invalid_handle);
    CHECK(h.service->diagnostic().memberships == before.memberships - 1);
    CHECK(h.service->diagnostic().private_keys == before.private_keys - 1);
    CHECK(h.service->diagnostic().trust_contexts == before.trust_contexts - 1);
    CHECK(h.service->diagnostic().public_record_bytes ==
          before.public_record_bytes - revoked_record.size - old_record.size);
    CHECK(h.service->recognize_device(owner, unrelated_trust, unrelated.device_public, current));
    // Deleting local credentials is not a cross-profile revocation broadcast.
    CHECK(h.service->recognize_device(observer, observer_trust, member.devices[0].public_key, current));
    const auto replacement = take(h.service->pin_authority(owner, expected));
    CHECK(replacement != self_trust);
    CHECK_FALSE(take(h.service->trust(owner, replacement)).has_record);
    rejected(h.service->recognize_device(owner, replacement, member.devices[0].public_key, current),
             gi::ErrorCode::unauthorized_device);
}

TEST_CASE("identity ID19 profile and private key admission rejects without partial publication") {
    gi::ServiceLimits limits;
    limits.profiles = 1;
    limits.private_keys = 2;
    Harness h(limits);
    const auto p = h.profile();
    const auto before = take(h.service->profile(p));
    const auto bytes = h.service->diagnostic().public_record_bytes;
    rejected(h.service->create_profile("2", "over budget", current), gi::ErrorCode::resource_exhausted);
    rejected(h.service->rotate_profile_device(p, before.devices[0].id, current),
             gi::ErrorCode::resource_exhausted);
    rejected(h.service->add_profile_device(p, current), gi::ErrorCode::resource_exhausted);
    CHECK(h.service->diagnostic().profiles == 1);
    CHECK(h.service->diagnostic().private_keys == 2);
    CHECK(h.service->diagnostic().public_record_bytes == bytes);
    CHECK(take(h.service->profile(p)).revision == before.revision);
    CHECK(h.crypto->diagnostic().live_secret_handles == 2);
    REQUIRE(h.service->revoke_profile_device(p, before.devices[0].id, current));
    CHECK(h.service->diagnostic().private_keys == 1);
    CHECK(h.service->add_profile_device(p, current));
    CHECK(h.service->diagnostic().private_keys == 2);
}

TEST_CASE("identity ID19 device membership trust and byte limits never evict accepted state") {
    SUBCASE("device ceiling allows replacing one device with staged headroom") {
        gi::ServiceLimits limits;
        limits.devices_per_authority = 1;
        Harness h(limits);
        const auto p = h.profile();
        const auto old = take(h.service->profile(p));
        rejected(h.service->add_profile_device(p, current), gi::ErrorCode::resource_exhausted);
        CHECK(h.service->rotate_profile_device(p, old.devices[0].id, current));
        CHECK(take(h.service->profile(p)).devices.size() == 1);
    }
    SUBCASE("membership ceiling preserves existing credential") {
        gi::ServiceLimits limits;
        limits.memberships_per_profile = 1;
        Harness h(limits);
        const auto p = h.profile();
        const auto ca = take(h.service->pin_community(p, h.community_key()));
        const auto cb = take(h.service->pin_community(p, h.community_key()));
        const auto m = take(h.service->prepare_membership(p, ca, "1", "A", current));
        rejected(h.service->prepare_membership(p, cb, "2", "B", current), gi::ErrorCode::resource_exhausted);
        CHECK(h.service->membership(p, m));
        CHECK(h.service->diagnostic().memberships == 1);
        CHECK(h.service->diagnostic().private_keys == 4);
    }
    SUBCASE("trust exhaustion cannot evict replay protection") {
        gi::ServiceLimits limits;
        limits.trust_contexts = 1;
        Harness h(limits);
        const auto p = h.profile();
        ExternalAuthority a(*h.crypto), b(*h.crypto);
        const auto trust = take(h.service->pin_authority(p, a.expected));
        REQUIRE(h.service->accept_record(p, trust, a.record(8).view(), current));
        rejected(h.service->pin_authority(p, b.expected), gi::ErrorCode::resource_exhausted);
        CHECK(take(h.service->trust(p, trust)).highest_revision == 8);
        CHECK(h.service->recognize_device(p, trust, a.device_public, current));
    }
    SUBCASE("record bytes rejected before retaining an accepted frame") {
        gi::ServiceLimits limits;
        limits.public_record_bytes = 230;
        Harness h(limits);
        const auto p = h.profile();
        ExternalAuthority authority(*h.crypto);
        const auto trust = take(h.service->pin_authority(p, authority.expected));
        rejected(h.service->accept_record(p, trust, authority.record().view(), current),
                 gi::ErrorCode::resource_exhausted);
        CHECK_FALSE(take(h.service->trust(p, trust)).has_record);
        CHECK(take(h.service->trust(p, trust)).highest_revision == 0);
        CHECK(h.service->diagnostic().public_record_bytes == 230);
    }
}

TEST_CASE("identity ID19 checked configuration ceilings reject overflowing or inconsistent limits") {
    ManualClock clock;
    auto crypto = take(gc::CryptoProvider::create());
    for (const auto field : {0, 1, 2, 3, 4, 5, 6}) {
        gi::ServiceLimits limits;
        const auto too_many = std::numeric_limits<std::size_t>::max();
        switch (field) {
        case 0:
            limits.profiles = too_many;
            break;
        case 1:
            limits.devices_per_authority = too_many;
            break;
        case 2:
            limits.memberships_per_profile = too_many;
            break;
        case 3:
            limits.private_keys = too_many;
            break;
        case 4:
            limits.trust_contexts = too_many;
            break;
        case 5:
            limits.public_record_bytes = too_many;
            break;
        case 6:
            limits.input_bytes = too_many;
            break;
        default:
            break;
        }
        CHECK_FALSE(gi::IdentityService::create(*crypto, clock, limits));
    }
    gi::ServiceLimits limits;
    limits.operation_timeout = std::chrono::milliseconds(0);
    CHECK_FALSE(gi::IdentityService::create(*crypto, clock, limits));
    limits.operation_timeout = std::chrono::milliseconds(5001);
    CHECK_FALSE(gi::IdentityService::create(*crypto, clock, limits));
    limits = {};
    limits.maximum_lifetime_seconds = gi::maximum_device_set_lifetime + 1;
    CHECK_FALSE(gi::IdentityService::create(*crypto, clock, limits));
    limits = {};
    limits.issued_lifetime_seconds = limits.maximum_lifetime_seconds + 1;
    CHECK_FALSE(gi::IdentityService::create(*crypto, clock, limits));
    CHECK(crypto->diagnostic().live_secret_handles == 0);
}

TEST_CASE("identity ID21 pre cancellation and terminal stop have no partial public or private effects") {
    Harness h;
    const auto p = h.profile();
    const auto before = take(h.service->profile(p));
    std::stop_source call;
    call.request_stop();
    rejected(h.service->create_profile("2", "cancelled", current, call.get_token()),
             gi::ErrorCode::cancelled);
    rejected(h.service->rotate_profile_device(p, before.devices[0].id, current, call.get_token()),
             gi::ErrorCode::cancelled);
    CHECK(h.service->diagnostic().private_keys == 2);
    CHECK(h.service->diagnostic().profiles == 1);
    CHECK(take(h.service->profile(p)).revision == before.revision);
    h.service->request_stop();
    rejected(h.service->create_profile("3", "stopped", current), gi::ErrorCode::cancelled);
    REQUIRE(h.service->close());
    REQUIRE(h.service->close());
    CHECK(h.service->diagnostic().state == gi::ServiceState::stopped);
    CHECK(h.service->diagnostic().private_keys == 0);
    CHECK(h.crypto->diagnostic().live_secret_handles == 0);
    CHECK_FALSE(h.service->profile(p));
}

TEST_CASE("identity ID23 strict bounded labels reject malformed UTF8 and controls before key generation") {
    Harness h;
    for (const std::string &alias : {std::string{}, std::string("a123"), std::string("123 4"),
                                     std::string(21, '1'), std::string("\xef\xbc\x91")}) {
        rejected(h.service->create_profile(alias, "display", current), gi::ErrorCode::invalid_argument);
    }
    for (const std::string &name :
         {std::string("\xc3\x28"), std::string("\xed\xa0\x80"), std::string("a\0b", 3),
          std::string("PRIVATE_DISPLAY_98765\nbreak"), std::string(257, 'x')}) {
        rejected(h.service->create_profile("123", name, current), gi::ErrorCode::invalid_argument);
    }
    CHECK(h.crypto->diagnostic().live_secret_handles == 0);
    CHECK(h.service->diagnostic().profiles == 0);
    CHECK(h.service->create_profile(std::string(20, '0'), std::string(256, 'x'), current));
    for (unsigned value = 0; value <= static_cast<unsigned>(gi::ErrorCode::crypto_failure); ++value) {
        const auto text = gi::describe(static_cast<gi::ErrorCode>(value));
        CHECK_FALSE(text.empty());
        CHECK(text.find("123") == std::string_view::npos);
        CHECK(text.find("PRIVATE_DISPLAY_98765") == std::string_view::npos);
    }
}

TEST_CASE("identity ID25 known authority and device keys cannot be reclassified into incompatible roles") {
    Harness h;
    const auto p = h.profile();
    const auto local = take(h.service->profile(p));
    rejected(h.service->pin_authority(p, {gi::AuthorityKind::profile, {}, local.devices[0].public_key}),
             gi::ErrorCode::wrong_scope);
    ExternalAuthority authority(*h.crypto), other(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    const auto other_trust = take(h.service->pin_authority(p, other.expected));
    REQUIRE(h.service->accept_record(p, trust, authority.record().view(), current));
    rejected(h.service->pin_authority(p, {gi::AuthorityKind::profile, {}, authority.device_public}),
             gi::ErrorCode::wrong_scope);
    gi::DeviceSet invalid_role;
    invalid_role.authority = other.expected;
    invalid_role.revision = 1;
    invalid_role.valid_from = 900;
    invalid_role.valid_until = 2000;
    invalid_role.device_count = 1;
    invalid_role.devices[0] = {authority.expected.public_key, 900, 2000};
    const auto frame = take(gi::sign_device_set(*h.crypto, other.key, invalid_role));
    rejected(h.service->accept_record(p, other_trust, frame.view(), current), gi::ErrorCode::wrong_scope);
    CHECK_FALSE(take(h.service->trust(p, other_trust)).has_record);
    CHECK(h.service->recognize_device(p, trust, authority.device_public, current));
    const auto community_key = h.community_key();
    REQUIRE(h.service->pin_community(p, community_key));
    invalid_role.devices[0].public_key = community_key;
    const auto community_as_device = take(gi::sign_device_set(*h.crypto, other.key, invalid_role));
    rejected(h.service->accept_record(p, other_trust, community_as_device.view(), current),
             gi::ErrorCode::wrong_scope);
}

TEST_CASE("identity ID26 explicit forgetting invalidates only its context and preserves owned keys") {
    Harness h;
    const auto p = h.profile(), other = h.profile();
    ExternalAuthority authority(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    REQUIRE(h.service->accept_record(p, trust, authority.record().view(), current));
    const auto keys = h.service->diagnostic().private_keys;
    REQUIRE(h.service->accept_record(p, trust, authority.record(2, 900, 2000, false).view(), current));
    CHECK(h.service->diagnostic().private_keys == keys);
    REQUIRE(h.service->forget_authority(p, trust));
    const auto replacement = take(h.service->pin_authority(p, authority.expected));
    CHECK(replacement != trust);
    rejected(h.service->trust(p, trust), gi::ErrorCode::invalid_handle);
    CHECK_FALSE(take(h.service->trust(p, replacement)).has_record);
    REQUIRE(h.service->delete_profile(p));
    CHECK(h.service->profile(other));
    CHECK(h.service->diagnostic().private_keys == 2);
    CHECK(h.service->diagnostic().trust_contexts == 0);
}

TEST_CASE("identity ownership tokens cannot keep keys alive after the service is destroyed") {
    ManualClock clock;
    auto crypto = take(gc::CryptoProvider::create());
    gi::ProfileHandle retained;
    {
        auto service = take(gi::IdentityService::create(*crypto, clock));
        retained = take(service->create_profile("1", "owned", current));
        CHECK(crypto->diagnostic().live_secret_handles == 2);
    }
    CHECK(crypto->diagnostic().live_secret_handles == 0);
    auto replacement = take(gi::IdentityService::create(*crypto, clock));
    rejected(replacement->profile(retained), gi::ErrorCode::invalid_handle);
}

TEST_CASE("identity membership generation and explicit community forgetting preserve scoped ownership") {
    Harness h;
    const auto p = h.profile();
    const auto community_key = h.community_key();
    const auto c = take(h.service->pin_community(p, community_key));
    const auto old_handle = take(h.service->prepare_membership(p, c, "1", "old", current));
    const auto old = take(h.service->membership(p, old_handle));
    REQUIRE(h.service->forget_community(p, c));
    CHECK(h.service->diagnostic().private_keys == 4);
    CHECK(h.service->membership(p, old_handle));
    REQUIRE(h.service->delete_membership(p, old_handle));
    rejected(h.service->prepare_membership(p, c, "1", "unpinned", current),
             gi::ErrorCode::untrusted_authority);
    REQUIRE(h.service->pin_community(p, community_key));
    const auto replacement = take(h.service->prepare_membership(p, c, "1", "new", current));
    CHECK(replacement != old_handle);
    CHECK(take(h.service->membership(p, replacement)).id != old.id);
    rejected(h.service->membership(p, old_handle), gi::ErrorCode::invalid_handle);
    rejected(h.service->rotate_membership_device(p, replacement, old.devices[0].id, current),
             gi::ErrorCode::unauthorized_device);
    CHECK(h.service->diagnostic().private_keys == 4);
}

TEST_CASE("identity UTC upper bound and maximum revisions cannot wrap or partially update state") {
    gi::ServiceLimits limits;
    limits.issued_lifetime_seconds = 1;
    Harness h(limits);
    const auto p = take(h.service->create_profile("1", "upper UTC", {gi::maximum_utc_seconds - 1, true}));
    const auto before = take(h.service->profile_record(p));
    rejected(h.service->renew_profile_record(p, {gi::maximum_utc_seconds, true}),
             gi::ErrorCode::time_untrusted);
    CHECK(take(h.service->profile_record(p)) == before);
    REQUIRE(h.service->revalidate_time(current));
    ExternalAuthority authority(*h.crypto);
    const auto trust = take(h.service->pin_authority(p, authority.expected));
    const auto maximum = std::numeric_limits<std::uint64_t>::max();
    const auto frame = authority.record(maximum);
    REQUIRE(h.service->accept_record(p, trust, frame.view(), current));
    REQUIRE(h.service->accept_record(p, trust, frame.view(), current));
    rejected(h.service->accept_record(p, trust, authority.record(maximum - 1).view(), current),
             gi::ErrorCode::stale_revision);
    CHECK(take(h.service->trust(p, trust)).highest_revision == maximum);
}
