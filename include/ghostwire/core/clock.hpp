#pragma once
#include "ghostwire/core/result.hpp"
#include <chrono>

namespace ghostwire::core {
// Injected clocks must be monotonic, thread-safe and outlive their context.
class Clock {
  public:
    using TimePoint = std::chrono::steady_clock::time_point;
    using Duration = std::chrono::steady_clock::duration;
    virtual ~Clock() = default;
    [[nodiscard]] virtual TimePoint now() const noexcept = 0;
};
class SteadyClock final : public Clock {
  public:
    [[nodiscard]] TimePoint now() const noexcept override { return std::chrono::steady_clock::now(); }
};

// One absolute deadline is shared by all shutdown hooks; it is never reset per service.
class Deadline {
  public:
    static Result<Deadline> after(const Clock &clock, std::chrono::milliseconds timeout);
    [[nodiscard]] Clock::TimePoint time_point() const noexcept { return end_; }
    [[nodiscard]] bool expired(const Clock &clock) const noexcept { return clock.now() >= end_; }
    [[nodiscard]] Clock::Duration remaining(const Clock &clock) const noexcept;

  private:
    explicit Deadline(Clock::TimePoint end) : end_(end) {}
    Clock::TimePoint end_;
};
inline constexpr auto default_shutdown_timeout = std::chrono::milliseconds(5000);
inline constexpr auto max_shutdown_timeout = std::chrono::milliseconds(60000);
} // namespace ghostwire::core
