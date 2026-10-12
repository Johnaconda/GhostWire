#include "ghostwire/core/clock.hpp"

namespace ghostwire::core {
Result<Deadline> Deadline::after(const Clock &clock, std::chrono::milliseconds timeout) {
    if (timeout.count() < 0 || timeout > max_shutdown_timeout)
        return Error{ErrorCode::configuration, "Shutdown timeout must be between 0 and 60000 ms.",
                     ErrorReason::invalid_budget};
    const auto delta = std::chrono::duration_cast<Clock::Duration>(timeout);
    const auto now = clock.now();
    const auto end = now > Clock::TimePoint::max() - delta ? Clock::TimePoint::max() : now + delta;
    return Deadline(end);
}
Clock::Duration Deadline::remaining(const Clock &clock) const noexcept {
    const auto now = clock.now();
    return now >= end_ ? Clock::Duration::zero() : end_ - now;
}
} // namespace ghostwire::core
