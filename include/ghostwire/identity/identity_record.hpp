#pragma once
#include "ghostwire/identity/identity_id.hpp"
#include <cstddef>
#include <span>

namespace ghostwire::identity {
inline constexpr std::size_t maximum_devices = 32;
inline constexpr std::size_t device_set_header_bytes = 118;
inline constexpr std::size_t device_set_entry_bytes = 48;
inline constexpr std::size_t maximum_device_set_bytes = 1718;
inline constexpr std::uint64_t maximum_utc_seconds = 253402300799;
inline constexpr std::uint64_t default_device_set_lifetime = 30 * 24 * 60 * 60;
inline constexpr std::uint64_t maximum_device_set_lifetime = 90 * 24 * 60 * 60;

// Explicit UTC from the trusted composition owner, unrelated to steady_clock.
// An omitted/default value fails closed. Clock rollback policy belongs to service.
struct VerificationTime {
    std::uint64_t seconds = 0;
    bool trusted = false;
    bool operator==(const VerificationTime &) const = default;
};
enum class AuthorityKind : std::uint8_t { profile = 1, membership = 2 };
struct ExpectedAuthority {
    AuthorityKind kind = AuthorityKind::profile;
    CommunityId scope{}; // Must be zero for a profile root.
    crypto::PublicKey public_key{};
    bool operator==(const ExpectedAuthority &) const = default;
};
struct DeviceEntry {
    crypto::PublicKey public_key{};
    std::uint64_t valid_from = 0;
    std::uint64_t valid_until = 0;
    bool operator==(const DeviceEntry &) const = default;
};
struct DeviceSet {
    ExpectedAuthority authority{};
    std::uint64_t revision = 0;
    std::uint64_t valid_from = 0;
    std::uint64_t valid_until = 0;
    std::array<DeviceEntry, maximum_devices> devices{};
    std::size_t device_count = 0;
    bool operator==(const DeviceSet &) const = default;
};
// Fixed storage prevents input-driven allocation. Only view() is encoded data;
// unused bytes are zero. Public values may cross threads as immutable copies.
struct DeviceSetFrame {
    std::array<std::uint8_t, maximum_device_set_bytes> bytes{};
    std::size_t size = 0;
    [[nodiscard]] std::span<const std::uint8_t> view() const noexcept {
        if (size > bytes.size())
            return {};
        return std::span(bytes).first(size);
    }
    bool operator==(const DeviceSetFrame &) const = default;
};

// A bounded, purpose-specific public-record primitive, not a generic signing
// oracle. The supplied authority key must match the record's authority exactly.
// Entries must already be sorted, unique and interval-valid; no normalization.
Result<DeviceSetFrame> sign_device_set(crypto::CryptoProvider &, const crypto::SigningKey &,
                                       const DeviceSet &, std::stop_token = {});
// Does not pin trust, accept a revision, authenticate a live peer or grant any
// action. Expected authority is selected independently by a trusted caller.
Result<DeviceSet> verify_device_set(crypto::CryptoProvider &, std::span<const std::uint8_t>,
                                    const ExpectedAuthority &, VerificationTime,
                                    std::uint64_t maximum_lifetime = maximum_device_set_lifetime,
                                    std::stop_token = {});
} // namespace ghostwire::identity
