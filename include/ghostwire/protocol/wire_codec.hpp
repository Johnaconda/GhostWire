#pragma once

#include "ghostwire/core/result.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stop_token>
#include <variant>
#include <vector>

namespace ghostwire::protocol {
// Wire v1.0 is a strictly closed public schema, not a security/session protocol.
inline constexpr std::size_t maximum_frame_bytes = 65536;
inline constexpr std::size_t maximum_device_set_bytes = 1718;
inline constexpr std::size_t minimum_device_set_bytes = 182;

// Error codes are fixed, do not contain caller data or raw frame bytes.
enum class ErrorCode : std::uint8_t {
    incomplete_frame, frame_limit, unsupported_version, unsupported_kind,
    unsupported_capability, noncanonical_encoding, malformed_frame,
    resource_limit, cancelled, deadline_exceeded, operation_busy
};
struct Error {
    ErrorCode code;
    bool operator==(const Error &) const = default;
};
template <typename T> using Result = core::Result<T, Error>;

struct Version {
    std::uint64_t major = 1;
    std::uint64_t minor = 0;
    bool operator==(const Version &) const = default;
};
struct VersionOffer {
    // v1.0 supports only one version and no registered capabilities.
    std::array<Version, 8> versions{{Version{1, 0}}};
    std::size_t count = 1;
    std::array<std::uint64_t, 32> capabilities{};
    std::size_t capability_count = 0;
    bool operator==(const VersionOffer &) const = default;
};
struct VersionSelect {
    Version selected{};
    std::array<std::uint64_t, 32> capabilities{};
    std::size_t capability_count = 0;
    bool operator==(const VersionSelect &) const = default;
};
struct PublicDeviceSet {
    // Public record bytes only. No signature verification, pinning, trust or
    // membership acceptance is performed by this library.
    std::vector<std::uint8_t> frame;
    bool operator==(const PublicDeviceSet &) const = default;
};
struct RoutingHint {
    std::array<std::uint8_t, 16> token{}; // Untrusted transient token, never identity.
    std::optional<std::uint8_t> hop_limit;
    bool operator==(const RoutingHint &) const = default;
};
struct Envelope {
    std::variant<VersionOffer, VersionSelect, PublicDeviceSet> body{VersionOffer{}};
    std::optional<RoutingHint> routing;
    bool operator==(const Envelope &) const = default;
};

// Caller-owned admission slot; no background queue and never global state.
// One owner must not call release while another thread executes an operation.
class Admission {
  public:
    Admission() = default;
    Admission(const Admission &) = delete;
    Admission &operator=(const Admission &) = delete;
    [[nodiscard]] bool try_acquire() noexcept { return !busy_.test_and_set(std::memory_order_acquire); }
    void release() noexcept { busy_.clear(std::memory_order_release); }
  private:
    std::atomic_flag busy_ = ATOMIC_FLAG_INIT;
};
struct Operation {
    std::stop_token stop{};
    std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::time_point::max();
    // May only tighten fixed ceilings, never relax them.
    std::size_t maximum_bytes = maximum_frame_bytes;
    std::size_t maximum_work = maximum_frame_bytes + 512;
};

// Each call must receive a bounded caller-owned admission slot. Results own
// bytes; failures have no output or profile/identity side effect.
[[nodiscard]] Result<std::vector<std::uint8_t>> encode_frame(const Envelope &, Admission &,
                                                              const Operation & = {});
[[nodiscard]] Result<Envelope> decode_frame(std::span<const std::uint8_t>, Admission &,
                                            const Operation & = {});
// Pure advisory selection: no authenticated handshake or downgrade guarantee.
[[nodiscard]] bool selected_was_offered(const VersionOffer &, const VersionSelect &) noexcept;
} // namespace ghostwire::protocol
