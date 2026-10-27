#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ghostwire/protocol/wire_codec.hpp"
#include "ghostwire/identity/identity_record.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <doctest/doctest.h>
#include <random>
#include <string_view>
#include <vector>

namespace {
namespace p = ghostwire::protocol;
namespace identity = ghostwire::identity;
using Bytes = std::vector<std::uint8_t>;
Bytes from_hex(std::string_view input) {
    REQUIRE(input.size() % 2 == 0);
    Bytes out;
    for (std::size_t i = 0; i < input.size(); i += 2) {
        const auto nibble = [](char c) -> std::uint8_t {
            if (c >= '0' && c <= '9') return static_cast<std::uint8_t>(c - '0');
            if (c >= 'a' && c <= 'f') return static_cast<std::uint8_t>(c - 'a' + 10);
            return static_cast<std::uint8_t>(c - 'A' + 10);
        };
        out.push_back(static_cast<std::uint8_t>(nibble(input[i]) * 16U + nibble(input[i + 1])));
    }
    return out;
}
void fails(const Bytes &data, p::ErrorCode expected) {
    p::Admission admission;
    const auto result = p::decode_frame(data, admission);
    REQUIRE_FALSE(result);
    CHECK(result.error().code == expected);
}
// These hex constants were independently authored in A05-P01, not produced
// from the C++ implementation under test.
constexpr std::string_view offer_hex = "00000010a400010100020103a200818201000180";
constexpr std::string_view select_hex = "0000000fa400010100020203a2008201000180";
struct Negative { std::string_view data; p::ErrorCode category; };
constexpr std::array negative_fixtures{
    Negative{"00000000", p::ErrorCode::frame_limit},
    Negative{"000000", p::ErrorCode::incomplete_frame},
    Negative{"00000010a400010100020103a2008182010001", p::ErrorCode::incomplete_frame},
    Negative{"00000010a400020100020103a200818201000180", p::ErrorCode::unsupported_version},
    Negative{"00000010a400010101020103a200818201000180", p::ErrorCode::unsupported_version},
    Negative{"00000010a400010100021403a200818201000180", p::ErrorCode::unsupported_kind},
    Negative{"00000012a500010100020103a2008182010001800001", p::ErrorCode::noncanonical_encoding},
    Negative{"00000010a401000001020103a200818201000180", p::ErrorCode::noncanonical_encoding},
    Negative{"00000011a40018010100020103a200818201000180", p::ErrorCode::noncanonical_encoding},
    Negative{"00000010a400010100020103a20081820100018000", p::ErrorCode::malformed_frame},
    Negative{"00000011bf00010100020103a200818201000180ff", p::ErrorCode::noncanonical_encoding},
    Negative{"00000011a400010100020103a20081820100018101", p::ErrorCode::unsupported_capability},
    Negative{"00010001", p::ErrorCode::frame_limit},
    Negative{"00000001a0", p::ErrorCode::malformed_frame},
    Negative{"00000012a500010100020103a2008182010001800500", p::ErrorCode::malformed_frame},
    Negative{"00000012a400010100020103a3008182010001800180", p::ErrorCode::noncanonical_encoding}
};
Bytes wrap(const Bytes &body) {
    REQUIRE(body.size() <= p::maximum_frame_bytes);
    Bytes framed{static_cast<std::uint8_t>((body.size() >> 24U) & 255U),
                 static_cast<std::uint8_t>((body.size() >> 16U) & 255U),
                 static_cast<std::uint8_t>((body.size() >> 8U) & 255U),
                 static_cast<std::uint8_t>(body.size() & 255U)};
    framed.insert(framed.end(), body.begin(), body.end());
    return framed;
}
} // namespace

TEST_CASE("frozen independent v1.0 goldens match exact encoded bytes and decoded values") {
    p::Admission slot;
    const p::Envelope offer{p::VersionOffer{}, std::nullopt};
    const p::Envelope select{p::VersionSelect{}, std::nullopt};
    for (const auto &entry : std::array{std::pair{offer, from_hex(offer_hex)}, std::pair{select, from_hex(select_hex)}}) {
        auto encoded = p::encode_frame(entry.first, slot);
        REQUIRE(encoded);
        CHECK(encoded.value() == entry.second);
        auto decoded = p::decode_frame(entry.second, slot);
        REQUIRE(decoded);
        CHECK(decoded.value() == entry.first);
    }
    CHECK(p::selected_was_offered(p::VersionOffer{}, p::VersionSelect{}));
    p::VersionOffer unoffered;
    unoffered.count = 0;
    CHECK_FALSE(p::selected_was_offered(unoffered, p::VersionSelect{}));
}

TEST_CASE("all 16 original design negative fixtures reject with pinned error categories") {
    for (const auto &entry : negative_fixtures) {
        CAPTURE(entry.data);
        fails(from_hex(entry.data), entry.category);
    }
}

TEST_CASE("routing is an untrusted optional exact 16-byte value with explicit hop ceiling") {
    p::Envelope msg;
    p::RoutingHint route;
    for (std::size_t i = 0; i < route.token.size(); ++i) route.token[i] = static_cast<std::uint8_t>(i);
    route.hop_limit = 16;
    msg.routing = route;
    p::Admission slot;
    auto encoded = p::encode_frame(msg, slot);
    REQUIRE(encoded);
    auto decoded = p::decode_frame(encoded.value(), slot);
    REQUIRE(decoded);
    CHECK(decoded.value() == msg);
    // Same kind, version and body, but different routing token -> never selects a principal.
    msg.routing->token[0] ^= 1;
    auto other = p::encode_frame(msg, slot);
    REQUIRE(other);
    CHECK(other.value() != encoded.value());
    msg.routing->hop_limit = 17;
    auto invalid = p::encode_frame(msg, slot);
    REQUIRE_FALSE(invalid);
    CHECK(invalid.error().code == p::ErrorCode::malformed_frame);
    auto badhop = encoded.value();
    badhop.back() = 17;
    fails(badhop, p::ErrorCode::malformed_frame);
    auto badtoken = encoded.value();
    // routing byte string header 0x50 changed to 0x4f (15, followed by 16 raw bytes).
    const auto it = std::find(badtoken.begin() + 4, badtoken.end(), static_cast<std::uint8_t>(0x50));
    REQUIRE(it != badtoken.end());
    *it = 0x4f;
    fails(badtoken, p::ErrorCode::malformed_frame);
}

TEST_CASE("public DeviceSet carrier copies source bytes unchanged but never accepts trust") {
    p::Admission slot;
    p::Envelope msg{p::PublicDeviceSet{Bytes(278, 0x3c)}, std::nullopt};
    auto frame = p::encode_frame(msg, slot);
    REQUIRE(frame);
    auto result = p::decode_frame(frame.value(), slot);
    REQUIRE(result);
    CHECK(result.value() == msg);
    CHECK(std::get<p::PublicDeviceSet>(result.value().body).frame == Bytes(278, 0x3c));
    // A04 independently rejects the opaque garbage, despite successful transport decode.
    auto provider = ghostwire::crypto::CryptoProvider::create();
    REQUIRE(provider);
    identity::ExpectedAuthority expected{};
    auto verified = identity::verify_device_set(*provider.value(),
        std::get<p::PublicDeviceSet>(result.value().body).frame, expected, {1700000010, true});
    CHECK_FALSE(verified);
    for (const auto n : {std::size_t{181}, std::size_t{1719}}) {
        msg.body = p::PublicDeviceSet{Bytes(n, 0xaa)};
        auto invalid = p::encode_frame(msg, slot);
        REQUIRE_FALSE(invalid);
        CHECK(invalid.error().code == p::ErrorCode::malformed_frame);
    }
    for (const auto n : {std::size_t{182}, std::size_t{1718}}) {
        msg.body = p::PublicDeviceSet{Bytes(n, 0xaa)};
        auto valid = p::encode_frame(msg, slot);
        REQUIRE(valid);
        CHECK(std::get<p::PublicDeviceSet>(p::decode_frame(valid.value(), slot).value().body).frame.size() == n);
    }
}

TEST_CASE("untrusted full frames, long lengths, invalid types and missing keys fail atomically") {
    Bytes data = from_hex(offer_hex);
    for (std::size_t n = 0; n < data.size(); ++n) {
        p::Admission slot;
        auto truncated = p::decode_frame(std::span(data).first(n), slot);
        CHECK_FALSE(truncated);
    }
    fails(from_hex("ffffffff"), p::ErrorCode::frame_limit);
    fails(from_hex("00010000") /* 65536 but incomplete */, p::ErrorCode::incomplete_frame);
    fails(from_hex("00000001ff"), p::ErrorCode::noncanonical_encoding);
    for (const auto invalid : {"00000001f6", "00000001c0", "0000000160", "0000000120"}) {
        fails(from_hex(invalid), p::ErrorCode::malformed_frame);
    }
    // Unsupported version pairs, reserved future security kinds, wrong body type.
    fails(from_hex("0000000fa400010100020203a2008201010180"), p::ErrorCode::unsupported_version);
    fails(from_hex("0000000fa400010100021403a2008201000180"), p::ErrorCode::unsupported_kind);
    fails(from_hex("0000000fa400010100020203a2008201000100"), p::ErrorCode::malformed_frame);
    // Definite byte-string length 65536 (0x5a 00010000) is rejected pre-allocation.
    fails(wrap(from_hex("a400010100020a03a1005a00010000")), p::ErrorCode::resource_limit);
}

TEST_CASE("bounded admission cancellation deadlines and resource budgets release on failure") {
    p::Admission slot;
    auto offer = from_hex(offer_hex);
    REQUIRE(slot.try_acquire());
    auto busy = p::decode_frame(offer, slot);
    REQUIRE_FALSE(busy);
    CHECK(busy.error().code == p::ErrorCode::operation_busy);
    slot.release();
    std::stop_source stop;
    stop.request_stop();
    p::Operation opts;
    opts.stop = stop.get_token();
    auto cancelled = p::decode_frame(offer, slot, opts);
    REQUIRE_FALSE(cancelled);
    CHECK(cancelled.error().code == p::ErrorCode::cancelled);
    opts.stop = {};
    opts.deadline = std::chrono::steady_clock::now() - std::chrono::seconds(1);
    auto expired = p::decode_frame(offer, slot, opts);
    REQUIRE_FALSE(expired);
    CHECK(expired.error().code == p::ErrorCode::deadline_exceeded);
    opts.deadline = std::chrono::steady_clock::time_point::max();
    opts.maximum_bytes = 8;
    auto too_big = p::decode_frame(offer, slot, opts);
    REQUIRE_FALSE(too_big);
    CHECK(too_big.error().code == p::ErrorCode::frame_limit);
    opts.maximum_bytes = p::maximum_frame_bytes;
    opts.maximum_work = 2;
    auto resource = p::decode_frame(offer, slot, opts);
    REQUIRE_FALSE(resource);
    CHECK(resource.error().code == p::ErrorCode::resource_limit);
    auto recovered = p::decode_frame(offer, slot);
    REQUIRE(recovered);
}

TEST_CASE("bounded deterministic input mutation corpus contains no unexpected successes or crashes") {
    p::Admission slot;
    std::mt19937 rng{0xA05B2026U};
    const Bytes original = from_hex(offer_hex);
    std::size_t rejected = 0;
    for (int round = 0; round < 15000; ++round) {
        Bytes mutated = original;
        const auto pos = static_cast<std::size_t>(rng() % mutated.size());
        mutated[pos] = static_cast<std::uint8_t>(rng() & 255U);
        auto decoded = p::decode_frame(mutated, slot);
        if (!decoded) ++rejected;
        else CHECK(decoded.value() == p::Envelope{});
    }
    CHECK(rejected > 13000);
}

TEST_CASE("genuine A04 Ed25519 DeviceSet remains byte-identical and verified only by A04") {
    auto provider = ghostwire::crypto::CryptoProvider::create();
    REQUIRE(provider);
    const Bytes seed = from_hex("9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60");
    auto signing_key = provider.value()->import_signing_seed(seed);
    REQUIRE(signing_key);
    auto root_public = provider.value()->public_key(signing_key.value());
    REQUIRE(root_public);
    identity::DeviceSet set;
    set.authority.kind = identity::AuthorityKind::profile;
    set.authority.public_key = root_public.value();
    set.revision = 1;
    set.valid_from = 1700000000;
    set.valid_until = 1700003600;
    auto signed_record = identity::sign_device_set(*provider.value(), signing_key.value(), set);
    REQUIRE(signed_record);
    const Bytes original(signed_record.value().view().begin(), signed_record.value().view().end());
    REQUIRE(original.size() == 182);
    p::Envelope envelope{p::PublicDeviceSet{original}, std::nullopt};
    p::Admission slot;
    auto wire = p::encode_frame(envelope, slot);
    REQUIRE(wire);
    auto recovered = p::decode_frame(wire.value(), slot);
    REQUIRE(recovered);
    auto carried = std::get<p::PublicDeviceSet>(recovered.value().body).frame;
    CHECK(carried == original);
    const auto verified = identity::verify_device_set(*provider.value(), carried, set.authority, {1700000010, true});
    REQUIRE(verified);
    CHECK(verified.value() == set);
    auto other = set.authority;
    other.public_key[0] ^= 1;
    CHECK_FALSE(identity::verify_device_set(*provider.value(), carried, other, {1700000010, true}));
    CHECK_FALSE(identity::verify_device_set(*provider.value(), carried, set.authority, {1700003600, true}));
}

TEST_CASE("nonpreferred collections, forbidden CBOR, unassigned keys and long frames reject") {
    auto valid = from_hex(offer_hex);
    Bytes body(valid.begin() + 4, valid.end());
    body.insert(body.begin(), 0xb8);
    body[1] = 4; // nonpreferred map length 4 = b8 04
    fails(wrap(body), p::ErrorCode::noncanonical_encoding);
    // A map key 3 encoded as 0x18 03 rather than one byte 03.
    body = Bytes(valid.begin() + 4, valid.end());
    body.insert(body.begin() + 7, 0x18);
    fails(wrap(body), p::ErrorCode::noncanonical_encoding);
    // The wire profile has no tag, text, negative int, simple value or float.
    for (std::uint8_t initial : std::array<std::uint8_t, 5>{0xc0, 0x60, 0x20, 0xf4, 0xf9})
        fails(wrap(Bytes{initial}), p::ErrorCode::malformed_frame);
    // Valid 65536 declared frame with complete body, but invalid CBOR.
    Bytes full(65540, 0);
    full[1] = 1;
    fails(full, p::ErrorCode::malformed_frame);
    // Empty byte-string claims with an unknown key are not silently ignored.
    body = Bytes(valid.begin() + 4, valid.end());
    body[0] = 0xa5; body.push_back(5); body.push_back(0);
    fails(wrap(body), p::ErrorCode::malformed_frame);
}

TEST_CASE("advisory version selection rejects impossible local offer configurations") {
    p::VersionOffer offered{};
    p::VersionSelect selected{};
    CHECK(p::selected_was_offered(offered, selected));
    offered.count = 2;
    offered.versions[1] = p::Version{2, 0};
    CHECK_FALSE(p::selected_was_offered(offered, selected));
    offered.count = 1;
    offered.versions[0] = p::Version{2, 0};
    CHECK_FALSE(p::selected_was_offered(offered, selected));
    offered.versions[0] = p::Version{};
    offered.capability_count = 1;
    CHECK_FALSE(p::selected_was_offered(offered, selected));
    offered.capability_count = 0;
    selected.capability_count = 1;
    CHECK_FALSE(p::selected_was_offered(offered, selected));
    selected.capability_count = 0;
    selected.selected = p::Version{1, 1};
    CHECK_FALSE(p::selected_was_offered(offered, selected));
}

TEST_CASE("encoder rejects insufficient work and byte budgets before constructing output") {
    p::Admission slot;
    p::Operation options;
    for (const auto &msg : std::array{p::Envelope{p::VersionOffer{}, std::nullopt},
                                      p::Envelope{p::VersionSelect{}, std::nullopt}}) {
        auto valid = p::encode_frame(msg, slot);
        REQUIRE(valid);
        const auto body_size = valid.value().size() - 4;
        options.maximum_bytes = body_size - 1;
        auto denied_bytes = p::encode_frame(msg, slot, options);
        REQUIRE_FALSE(denied_bytes);
        CHECK(denied_bytes.error().code == p::ErrorCode::frame_limit);
        options.maximum_bytes = body_size;
        options.maximum_work = valid.value().size() - 1;
        auto denied_work = p::encode_frame(msg, slot, options);
        REQUIRE_FALSE(denied_work);
        CHECK(denied_work.error().code == p::ErrorCode::resource_limit);
        options.maximum_work = valid.value().size();
        auto accepted = p::encode_frame(msg, slot, options);
        REQUIRE(accepted);
        CHECK(accepted.value() == valid.value());
        options = {};
    }
}

TEST_CASE("all public-record CBOR length-width boundaries and routing variants have exact preflight sizes") {
    p::Admission slot;
    for (const auto n : {std::size_t{182}, std::size_t{254}, std::size_t{255},
                         std::size_t{256}, std::size_t{257}, std::size_t{1718}}) {
        p::Envelope msg{p::PublicDeviceSet{Bytes(n, 0x54)}, std::nullopt};
        for (int variant = 0; variant < 3; ++variant) {
            if (variant > 0) {
                p::RoutingHint hint;
                hint.token.fill(0x7a);
                if (variant == 2) hint.hop_limit = 0;
                msg.routing = hint;
            }
            const std::size_t expected_body = n + (n <= 255 ? 12U : 13U) +
                                               (variant == 0 ? 0U : variant == 1 ? 20U : 22U);
            auto frame = p::encode_frame(msg, slot);
            REQUIRE(frame);
            CHECK(frame.value().size() == expected_body + 4U);
            auto roundtrip = p::decode_frame(frame.value(), slot);
            REQUIRE(roundtrip);
            CHECK(roundtrip.value() == msg);
            p::Operation low;
            low.maximum_work = expected_body + 3U;
            auto limited = p::encode_frame(msg, slot, low);
            REQUIRE_FALSE(limited);
            CHECK(limited.error().code == p::ErrorCode::resource_limit);
            low.maximum_work = expected_body + 4U;
            auto restored = p::encode_frame(msg, slot, low);
            REQUIRE(restored);
            CHECK(restored.value() == frame.value());
        }
    }
}


TEST_CASE("resource review checks frame work ceiling, cancellation, and lease recovery") {
    p::Admission slot;
    p::RoutingHint route;
    route.token.fill(0x42);
    route.hop_limit = 16;
    const std::array<p::Envelope, 4> fixtures{
        p::Envelope{p::VersionOffer{}, std::nullopt},
        p::Envelope{p::VersionSelect{}, route},
        p::Envelope{p::PublicDeviceSet{Bytes(p::minimum_device_set_bytes, 0x3a)}, std::nullopt},
        p::Envelope{p::PublicDeviceSet{Bytes(p::maximum_device_set_bytes, 0x7b)}, route},
    };
    for (const auto &msg : fixtures) {
        const auto wire = p::encode_frame(msg, slot);
        REQUIRE(wire);
        const auto original = wire.value();
        const auto body_size = wire.value().size() - 4U;
        p::Operation opts;
        opts.maximum_bytes = body_size - 1U;
        const auto byte_limited = p::decode_frame(original, slot, opts);
        REQUIRE_FALSE(byte_limited);
        CHECK(byte_limited.error().code == p::ErrorCode::frame_limit);
        opts.maximum_bytes = body_size;
        // Discover the precise fixed-parser charge ceiling. Every budget
        // below the first successful charge must fail without publishing a
        // partial output, and admission must remain reusable after each call.
        bool succeeded = false;
        for (std::size_t budget = 0; budget <= original.size() + 512U; ++budget) {
            opts.maximum_work = budget;
            auto decoded = p::decode_frame(original, slot, opts);
            if (decoded) {
                CHECK(decoded.value() == msg);
                succeeded = true;
                break;
            }
            CHECK(decoded.error().code == p::ErrorCode::resource_limit);
        }
        CHECK(succeeded);
        opts.maximum_work = p::maximum_frame_bytes + 512;
        std::stop_source stop;
        stop.request_stop();
        opts.stop = stop.get_token();
        auto cancelled = p::decode_frame(original, slot, opts);
        REQUIRE_FALSE(cancelled);
        CHECK(cancelled.error().code == p::ErrorCode::cancelled);
        const auto cancelled_encode = p::encode_frame(msg, slot, opts);
        REQUIRE_FALSE(cancelled_encode);
        CHECK(cancelled_encode.error().code == p::ErrorCode::cancelled);
        opts.stop = {};
        opts.deadline = std::chrono::steady_clock::now() - std::chrono::seconds(1);
        auto expired = p::decode_frame(original, slot, opts);
        REQUIRE_FALSE(expired);
        CHECK(expired.error().code == p::ErrorCode::deadline_exceeded);
        auto expired_encode = p::encode_frame(msg, slot, opts);
        REQUIRE_FALSE(expired_encode);
        CHECK(expired_encode.error().code == p::ErrorCode::deadline_exceeded);
        CHECK(original == wire.value());
        auto recovered = p::decode_frame(original, slot);
        REQUIRE(recovered);
        CHECK(recovered.value() == msg);
    }
}
