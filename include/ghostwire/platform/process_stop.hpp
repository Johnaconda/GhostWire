#pragma once
#include "ghostwire/core/result.hpp"
#include <memory>
#include <stop_token>

namespace ghostwire::platform {
enum class ProcessStopEvent { none, interrupt, terminate };
// Explicitly owned, single active OS registration. Native callbacks only latch
// lock-free state. dispatch() requests standard cancellation on the OWNER thread;
// call it at event-loop/safe lifecycle boundaries. No hidden dispatcher thread.
class ProcessStopBridge {
  public:
    static core::Result<std::unique_ptr<ProcessStopBridge>> install();
    ~ProcessStopBridge() noexcept;
    ProcessStopBridge(const ProcessStopBridge &) = delete;
    ProcessStopBridge &operator=(const ProcessStopBridge &) = delete;
    core::Result<ProcessStopEvent> dispatch();
    core::Result<void> close(); // Idempotent; restores previous POSIX dispositions.
    [[nodiscard]] std::stop_token stop_token() const noexcept;

  private:
    struct Impl;
    explicit ProcessStopBridge(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> impl_;
};
} // namespace ghostwire::platform
