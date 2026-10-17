#include "ghostwire/identity/identity_record.hpp"
#include <algorithm>
#include <concepts>
#include <doctest/doctest.h>
#include <limits>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {
using namespace ghostwire::identity;
namespace crypto = ghostwire::crypto;

template <typename Left, typename Right>
concept CrossComparable = requires(Left left, Right right) {
    { left == right } -> std::convertible_to<bool>;
};
static_assert(!std::is_convertible_v<ProfileIdentityId, CommunityId>);
static_assert(!CrossComparable<ProfileIdentityId, CommunityId>);
static_assert(!CrossComparable<ProfileDeviceId, MembershipDeviceId>);

std::vector<std::uint8_t> hex(std::string_view text) {
    const auto nibble = [](char c) -> std::uint8_t {
        return static_cast<std::uint8_t>(c <= '9' ? c - '0' : c - 'a' + 10);
    };
    REQUIRE(text.size() % 2 == 0);
    std::vector<std::uint8_t> bytes(text.size() / 2);
    for (std::size_t i = 0; i < bytes.size(); ++i)
        bytes[i] = static_cast<std::uint8_t>((nibble(text[i * 2]) << 4U) | nibble(text[i * 2 + 1]));
    return bytes;
}
crypto::PublicKey public_key(std::string_view text) {
    const auto bytes = hex(text);
    REQUIRE(bytes.size() == crypto::public_key_bytes);
    crypto::PublicKey key{};
    std::copy(bytes.begin(), bytes.end(), key.begin());
    return key;
}
std::unique_ptr<crypto::CryptoProvider> provider(crypto::Limits limits = {}) {
    auto created = crypto::CryptoProvider::create(limits);
    REQUIRE(created);
    return std::move(created).value();
}
template <typename T> void error(const Result<T> &result, ErrorCode expected) {
    REQUIRE_FALSE(result);
    CHECK(result.error().code == expected);
}
constexpr std::string_view fixture_seed = "9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60";
constexpr std::string_view fixture_root = "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a";
constexpr std::string_view fixture_device_a =
    "3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c";
constexpr std::string_view fixture_device_b =
    "fc51cd8e6218a1a38da47ed00230f0580816ed13ba3303ac5deb911548908025";

// Independent answer: manually concatenated ASCII/hex fields signed with Python
// cryptography 50.0.2 Ed25519PrivateKey and an RFC8032 public test seed. This full
// frame is fixed data, not assembled using the production serializer under test.
constexpr std::string_view fixture_frame = "47686f7374576972652e4465766963655365742e7631000001000102"
                                           "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
                                           "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a"
                                           "0102030405060708000000006553f100000000006553ff100002"
                                           "3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c"
                                           "000000006553f101000000006553ff0f"
                                           "fc51cd8e6218a1a38da47ed00230f0580816ed13ba3303ac5deb911548908025"
                                           "000000006553f102000000006553ff0e"
                                           "1cb5a92c6c1a1dc1c9f75b123185b995c6923cbcd180c9015d0bb8299ea00ec9"
                                           "09725831734a05e518f0721fefb5658533e7cf5632c25d956fa096b1527e0605";

DeviceSet fixture_set() {
    DeviceSet set;
    set.authority.kind = AuthorityKind::membership;
    for (std::size_t i = 0; i < 32; ++i)
        set.authority.scope.bytes[i] = static_cast<std::uint8_t>(i);
    set.authority.public_key = public_key(fixture_root);
    set.revision = 0x0102030405060708;
    set.valid_from = 1700000000;
    set.valid_until = 1700003600;
    set.device_count = 2;
    set.devices[0] = {public_key(fixture_device_a), 1700000001, 1700003599};
    set.devices[1] = {public_key(fixture_device_b), 1700000002, 1700003598};
    return set;
}
void resign(crypto::CryptoProvider &crypto, const crypto::SigningKey &key, std::vector<std::uint8_t> &bytes) {
    REQUIRE(bytes.size() >= 64);
    auto signature = crypto.sign_public(key, std::span(bytes).first(bytes.size() - 64));
    REQUIRE(signature);
    std::copy(signature.value().begin(), signature.value().end(), bytes.end() - 64);
}
} // namespace

TEST_CASE("identity records typed IDs match independent domain separated SHA256 answers") {
    auto crypto = provider();
    const auto root = public_key(fixture_root);
    auto profile = derive_profile_id(*crypto, root);
    auto community = derive_community_id(*crypto, root);
    REQUIRE(profile);
    REQUIRE(community);
    auto membership = derive_membership_id(*crypto, community.value(), root);
    REQUIRE(membership);
    auto profile_device = derive_profile_device_id(*crypto, profile.value(), root);
    auto membership_device = derive_membership_device_id(*crypto, membership.value(), root);
    REQUIRE(profile_device);
    REQUIRE(membership_device);
    // Python hashlib over literal domain + NUL + fixed-width public fields.
    CHECK(profile.value().bytes ==
          public_key("d5729ab712e7e260caad18eceb39dbbde1a17e16b6396a326b117eb7768bf47e"));
    CHECK(community.value().bytes ==
          public_key("e72030ab9fb716d3c0be26e8622a4ea097b4f0d853e3038e3b1267e92596e57e"));
    CHECK(membership.value().bytes ==
          public_key("e350bccfeba21e25c18dfa0a1170ba5ab37e273066894e2453d028131ac639fb"));
    CHECK(profile_device.value().bytes ==
          public_key("d41a364be04e6b154a59212811e7701b798a2d022d93ca205a47ce308a4c7b5f"));
    CHECK(membership_device.value().bytes ==
          public_key("33bafae14a8df60082e49b1def1d52544a0c50768c433b8bdb3c8de68375ec60"));
    auto other_scope = community.value();
    other_scope.bytes[31] ^= 1;
    auto other = derive_membership_id(*crypto, other_scope, root);
    REQUIRE(other);
    CHECK(other.value() != membership.value());
    std::stop_source stop;
    stop.request_stop();
    error(derive_profile_id(*crypto, root, stop.get_token()), ErrorCode::cancelled);
    error(derive_community_id(*crypto, root, stop.get_token()), ErrorCode::cancelled);
    error(derive_membership_id(*crypto, community.value(), root, stop.get_token()), ErrorCode::cancelled);
    error(derive_profile_device_id(*crypto, profile.value(), root, stop.get_token()), ErrorCode::cancelled);
    error(derive_membership_device_id(*crypto, membership.value(), root, stop.get_token()),
          ErrorCode::cancelled);
}

TEST_CASE("identity records canonical bytes and pure Ed25519 match an independent full frame") {
    auto crypto = provider();
    const auto set = fixture_set();
    const auto expected = hex(fixture_frame);
    REQUIRE(expected.size() == 278);
    auto verified = verify_device_set(*crypto, expected, set.authority, {1700000010, true});
    REQUIRE(verified);
    CHECK(verified.value() == set);
    auto key = crypto->import_signing_seed(hex(fixture_seed));
    REQUIRE(key);
    auto generated = sign_device_set(*crypto, key.value(), set);
    REQUIRE(generated);
    CHECK(generated.value().size == expected.size());
    CHECK(std::equal(expected.begin(), expected.end(), generated.value().view().begin()));
    CHECK(crypto->diagnostic().live_secret_handles == 1);
}

TEST_CASE("identity records reject every truncation every changed byte and trailing bytes") {
    auto crypto = provider();
    const auto set = fixture_set();
    const auto good = hex(fixture_frame);
    for (std::size_t length = 0; length < good.size(); ++length) {
        CAPTURE(length);
        CHECK_FALSE(
            verify_device_set(*crypto, std::span(good).first(length), set.authority, {1700000010, true}));
    }
    for (std::size_t i = 0; i < good.size(); ++i) {
        CAPTURE(i);
        auto changed = good;
        changed[i] ^= 1;
        CHECK_FALSE(verify_device_set(*crypto, changed, set.authority, {1700000010, true}));
    }
    for (const std::size_t extra : {1U, 48U, 8192U}) {
        auto changed = good;
        changed.resize(good.size() + extra);
        error(verify_device_set(*crypto, changed, set.authority, {1700000010, true}),
              ErrorCode::malformed_record);
    }
    CHECK(crypto->diagnostic().live_secret_handles == 0);
}

TEST_CASE("identity records reject alternate version suite purpose scope key and byte order") {
    auto crypto = provider();
    const auto set = fixture_set();
    auto bytes = hex(fixture_frame);
    bytes[24] = 2;
    error(verify_device_set(*crypto, bytes, set.authority, {1700000010, true}),
          ErrorCode::unsupported_version);
    bytes = hex(fixture_frame);
    bytes[26] = 2;
    error(verify_device_set(*crypto, bytes, set.authority, {1700000010, true}), ErrorCode::unsupported_suite);
    bytes = hex(fixture_frame);
    std::swap(bytes[23], bytes[24]);
    error(verify_device_set(*crypto, bytes, set.authority, {1700000010, true}),
          ErrorCode::unsupported_version);
    bytes = hex(fixture_frame);
    bytes[27] = 3;
    error(verify_device_set(*crypto, bytes, set.authority, {1700000010, true}), ErrorCode::malformed_record);
    auto wrong = set.authority;
    wrong.kind = AuthorityKind::profile;
    wrong.scope = {};
    error(verify_device_set(*crypto, hex(fixture_frame), wrong, {1700000010, true}), ErrorCode::wrong_scope);
    wrong = set.authority;
    wrong.scope.bytes[0] ^= 1;
    error(verify_device_set(*crypto, hex(fixture_frame), wrong, {1700000010, true}), ErrorCode::wrong_scope);
    wrong = set.authority;
    wrong.public_key = public_key(fixture_device_a);
    error(verify_device_set(*crypto, hex(fixture_frame), wrong, {1700000010, true}),
          ErrorCode::untrusted_authority);
    bytes = hex(fixture_frame);
    std::swap(bytes[116], bytes[117]);
    error(verify_device_set(*crypto, bytes, set.authority, {1700000010, true}), ErrorCode::malformed_record);
}

TEST_CASE("identity records signed noncanonical sets are still rejected") {
    auto crypto = provider();
    const auto set = fixture_set();
    auto key = crypto->import_signing_seed(hex(fixture_seed));
    REQUIRE(key);
    const auto expect_malformed = [&](std::vector<std::uint8_t> bytes) {
        resign(*crypto, key.value(), bytes);
        error(verify_device_set(*crypto, bytes, set.authority, {1700000010, true}),
              ErrorCode::malformed_record);
    };
    auto bytes = hex(fixture_frame);
    std::copy_n(bytes.begin() + 118, 32, bytes.begin() + 166); // Duplicate device.
    expect_malformed(bytes);
    bytes = hex(fixture_frame);
    std::swap_ranges(bytes.begin() + 118, bytes.begin() + 166, bytes.begin() + 166); // Unsorted.
    expect_malformed(bytes);
    bytes = hex(fixture_frame);
    std::fill(bytes.begin() + 92, bytes.begin() + 100, 0); // Revision zero.
    expect_malformed(bytes);
    bytes = hex(fixture_frame);
    std::copy_n(bytes.begin() + 100, 8, bytes.begin() + 108); // Empty outer interval.
    expect_malformed(bytes);
    bytes = hex(fixture_frame);
    std::fill(bytes.begin() + 108, bytes.begin() + 116, 0xff); // Overflowing timestamp.
    expect_malformed(bytes);
    bytes = hex(fixture_frame);
    std::copy_n(bytes.begin() + 158, 8, bytes.begin() + 150); // Empty device interval.
    expect_malformed(bytes);
    bytes = hex(fixture_frame);
    std::fill(bytes.begin() + 150, bytes.begin() + 158, 0); // Device starts before set.
    expect_malformed(bytes);
    bytes = hex(fixture_frame);
    std::fill(bytes.begin() + 158, bytes.begin() + 166, 0xff); // Device ends after set.
    expect_malformed(bytes);
    bytes = hex(fixture_frame);
    bytes[117] = 33;
    expect_malformed(bytes);
    bytes = hex(fixture_frame);
    bytes[27] = 1; // Profile cannot carry a community scope, even if signed.
    expect_malformed(bytes);
}

TEST_CASE("identity records empty and maximum sets are bounded and authority roles cannot be reused") {
    auto crypto = provider();
    auto key = crypto->import_signing_seed(hex(fixture_seed));
    REQUIRE(key);
    auto set = fixture_set();
    set.authority.kind = AuthorityKind::profile;
    set.authority.scope = {};
    set.device_count = 0;
    set.devices = {};
    auto empty = sign_device_set(*crypto, key.value(), set);
    REQUIRE(empty);
    CHECK(empty.value().size == 182);
    auto verified = verify_device_set(*crypto, empty.value().view(), set.authority, {1700000010, true});
    REQUIRE(verified);
    CHECK(verified.value().device_count == 0);
    set.device_count = maximum_devices;
    for (std::size_t i = 0; i < maximum_devices; ++i) {
        set.devices[i].public_key[0] = static_cast<std::uint8_t>(i);
        set.devices[i].valid_from = set.valid_from;
        set.devices[i].valid_until = set.valid_until;
    }
    auto maximum = sign_device_set(*crypto, key.value(), set);
    REQUIRE(maximum);
    CHECK(maximum.value().size == 1718);
    CHECK(verify_device_set(*crypto, maximum.value().view(), set.authority, {1700000010, true}));
    ++set.device_count;
    error(sign_device_set(*crypto, key.value(), set), ErrorCode::malformed_record);
    set.device_count = 1;
    set.devices[0].public_key = set.authority.public_key;
    error(sign_device_set(*crypto, key.value(), set), ErrorCode::malformed_record);
    set = fixture_set();
    set.authority.public_key = public_key(fixture_device_a);
    set.device_count = 0;
    error(sign_device_set(*crypto, key.value(), set), ErrorCode::untrusted_authority);
    DeviceSetFrame invalid;
    invalid.size = std::numeric_limits<std::size_t>::max();
    CHECK(invalid.view().empty());
}

TEST_CASE("identity records exact UTC validity boundaries and lifetime caps fail closed") {
    auto crypto = provider();
    const auto set = fixture_set();
    const auto frame = hex(fixture_frame);
    error(verify_device_set(*crypto, frame, set.authority, {}), ErrorCode::time_untrusted);
    error(verify_device_set(*crypto, frame, set.authority, {1700000000, false}), ErrorCode::time_untrusted);
    error(verify_device_set(*crypto, frame, set.authority, {maximum_utc_seconds + 1, true}),
          ErrorCode::time_untrusted);
    error(verify_device_set(*crypto, frame, set.authority, {set.valid_from - 1, true}),
          ErrorCode::not_yet_valid);
    CHECK(verify_device_set(*crypto, frame, set.authority, {set.valid_from, true}));
    CHECK(verify_device_set(*crypto, frame, set.authority, {set.valid_until - 1, true}));
    error(verify_device_set(*crypto, frame, set.authority, {set.valid_until, true}), ErrorCode::expired);
    error(verify_device_set(*crypto, frame, set.authority, {set.valid_until + 1, true}), ErrorCode::expired);
    error(verify_device_set(*crypto, frame, set.authority, {1700000010, true}, 3599),
          ErrorCode::malformed_record);
    CHECK(verify_device_set(*crypto, frame, set.authority, {1700000010, true}, 3600));
    error(verify_device_set(*crypto, frame, set.authority, {1700000010, true}, 0),
          ErrorCode::invalid_argument);
    error(
        verify_device_set(*crypto, frame, set.authority, {1700000010, true}, maximum_device_set_lifetime + 1),
        ErrorCode::invalid_argument);
    auto key = crypto->import_signing_seed(hex(fixture_seed));
    REQUIRE(key);
    auto edge = set;
    edge.device_count = 0;
    edge.devices = {};
    edge.revision = std::numeric_limits<std::uint64_t>::max();
    edge.valid_from = maximum_utc_seconds - 3600;
    edge.valid_until = maximum_utc_seconds;
    auto signed_edge = sign_device_set(*crypto, key.value(), edge);
    REQUIRE(signed_edge);
    CHECK(verify_device_set(*crypto, signed_edge.value().view(), edge.authority,
                            {maximum_utc_seconds - 1, true}));
    error(verify_device_set(*crypto, signed_edge.value().view(), edge.authority, {maximum_utc_seconds, true}),
          ErrorCode::expired);
    edge.valid_until = std::numeric_limits<std::uint64_t>::max();
    error(sign_device_set(*crypto, key.value(), edge), ErrorCode::malformed_record);
    edge.valid_from = 0;
    edge.valid_until = maximum_device_set_lifetime + 1;
    error(sign_device_set(*crypto, key.value(), edge), ErrorCode::malformed_record);
}

TEST_CASE("identity records cancellation wrong signer and provider bounds preserve explicit ownership") {
    auto crypto = provider();
    const auto set = fixture_set();
    auto key = crypto->import_signing_seed(hex(fixture_seed));
    REQUIRE(key);
    std::stop_source stop;
    stop.request_stop();
    error(sign_device_set(*crypto, key.value(), set, stop.get_token()), ErrorCode::cancelled);
    error(verify_device_set(*crypto, hex(fixture_frame), set.authority, {1700000010, true},
                            maximum_device_set_lifetime, stop.get_token()),
          ErrorCode::cancelled);
    error(sign_device_set(*crypto, crypto::SigningKey{}, set), ErrorCode::crypto_failure);
    CHECK(crypto->diagnostic().live_secret_handles == 1);
    CHECK(key.value().valid());
    crypto::Limits limits;
    limits.message_bytes = 32;
    auto bounded = provider(limits);
    error(sign_device_set(*bounded, key.value(), set), ErrorCode::crypto_failure);
    error(verify_device_set(*bounded, hex(fixture_frame), set.authority, {1700000010, true}),
          ErrorCode::crypto_failure);
    CHECK(bounded->diagnostic().live_secret_handles == 0);
    crypto->request_stop();
    error(sign_device_set(*crypto, key.value(), set), ErrorCode::cancelled);
    CHECK(key.value().valid()); // Public record processing never destroys owned keys.
}
