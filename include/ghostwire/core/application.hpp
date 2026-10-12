#pragma once
#include "ghostwire/core/application_lifecycle.hpp"
#include "ghostwire/core/service_manager.hpp"

namespace ghostwire::core {
struct ApplicationOptions {
    ResourcePolicy resources;
    std::chrono::milliseconds shutdown_timeout = default_shutdown_timeout;
};
inline constexpr ServiceId application_paths_service{1};
// Lifecycle/state/context graph methods are owner-thread only. request_stop()
// and stop_token() are thread-safe. Logger and an injected clock are borrowed.
class Application {
  public:
    Application(Role role, Config config, Paths paths, Logger &logger, ApplicationOptions options = {});
    Application(Role role, Config config, Paths paths, Logger &logger, const Clock &clock,
                ApplicationOptions options = {});
    ~Application() noexcept;
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;
    Result<void> start();
    Result<void> shutdown();
    Result<void> shutdown(std::chrono::milliseconds timeout);
    Result<void> add_service(ServiceId id, std::unique_ptr<Service> service,
                             std::span<const ServiceId> dependencies = {});
    void request_stop() noexcept;
    [[nodiscard]] std::stop_token stop_token() const noexcept;
    [[nodiscard]] ApplicationState state() const noexcept;
    [[nodiscard]] const ApplicationContext &context() const noexcept { return context_; }
    [[nodiscard]] ServiceSnapshot services() const { return services_.snapshot(); }
    [[nodiscard]] const ShutdownReport &shutdown_report() const noexcept {
        return services_.shutdown_report();
    }

  private:
    Application(Role role, Config config, Paths paths, Logger &logger, const Clock *clock,
                ApplicationOptions options);
    SteadyClock default_clock_;
    std::stop_source stop_;
    ResourceAdmission admission_;
    ApplicationContext context_;
    ServiceManager services_; // Destroyed before context/admission/clock.
    const ApplicationOptions options_;
    ApplicationState state_ = ApplicationState::created;
    bool startup_failed_ = false, logged_start_ = false, logged_stop_ = false;
    std::optional<Error> logging_error_;
};
} // namespace ghostwire::core
