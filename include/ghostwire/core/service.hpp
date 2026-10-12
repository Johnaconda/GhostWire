#pragma once
#include "ghostwire/core/application_context.hpp"
#include <cstdint>

namespace ghostwire::core {
struct ServiceId {
    std::uint32_t value = 0; // Nonzero, local, developer-assigned; not an authority token.
    bool operator==(const ServiceId &) const = default;
};
struct ShutdownContext {
    const Clock &clock;
    Deadline deadline;
    std::stop_token stop;
    [[nodiscard]] Clock::Duration remaining() const noexcept { return deadline.remaining(clock); }
};
class Service {
  public:
    virtual ~Service() = default;
    // Owner-thread hooks. start may borrow context until destruction. Every
    // attempted start (even a failure/throw) is paired with stop for partial cleanup.
    virtual Result<void> start(const ApplicationContext &context) = 0;
    // Success means ALL work/completions are joined/drained and leases released.
    // Failure means still owned; stop may be retried. At zero remaining time do
    // only nonblocking cleanup. Never detach; destructor remains a final RAII guard.
    virtual Result<void> stop(const ShutdownContext &context) = 0;
};
} // namespace ghostwire::core
