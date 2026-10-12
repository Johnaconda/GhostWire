#include "ghostwire/core/application.hpp"
#include "ghostwire/core/version.hpp"
#include "ghostwire/platform/platform_info.hpp"
#include <stdexcept>
#include <utility>

namespace ghostwire::core {
namespace {
// Real A00 filesystem initialization now participates in the same lifecycle as
// future services. Empty directories remain on disk; shutdown never deletes them.
class ApplicationPaths final : public Service {
  public:
    Result<void> start(const ApplicationContext &context) override {
        return initialize_paths(context.paths());
    }
    Result<void> stop(const ShutdownContext &) override { return {}; }
};
} // namespace
Application::Application(Role role, Config config, Paths paths, Logger &logger, ApplicationOptions options)
    : Application(role, std::move(config), std::move(paths), logger, nullptr, options) {}
Application::Application(Role role, Config config, Paths paths, Logger &logger, const Clock &clock,
                         ApplicationOptions options)
    : Application(role, std::move(config), std::move(paths), logger, &clock, options) {}
Application::Application(Role role, Config config, Paths paths, Logger &logger, const Clock *clock,
                         ApplicationOptions options)
    : admission_(
          resource_budget(config.limits, platform::platform_info().logical_processors, options.resources)),
      context_(role, std::move(config), std::move(paths), logger, clock ? *clock : default_clock_, admission_,
               stop_.get_token()),
      services_(context_, stop_, options.shutdown_timeout), options_(options) {
    const auto registered = services_.add(application_paths_service, std::make_unique<ApplicationPaths>());
    if (!registered)
        throw std::logic_error("Cannot register application filesystem lifecycle");
}
Application::~Application() noexcept {
    try {
        (void)shutdown();
    } catch (...) {
        request_stop();
    }
}
Result<void> Application::add_service(ServiceId id, std::unique_ptr<Service> service,
                                      std::span<const ServiceId> dependencies) {
    if (state_ != ApplicationState::created)
        return Error{ErrorCode::internal, "Application registration is closed.", ErrorReason::invalid_state};
    return services_.add(id, std::move(service), dependencies);
}
Result<void> Application::start() {
    if (state_ != ApplicationState::created)
        return Error{ErrorCode::internal, "Application cannot be started twice.", ErrorReason::invalid_state};
    state_ = ApplicationState::starting;
    auto valid = validate_config(context_.config());
    if (!valid) {
        startup_failed_ = true;
        state_ = ApplicationState::failed;
        request_stop();
        return valid.error();
    }
    auto started = services_.start();
    if (!started) {
        startup_failed_ = true;
        state_ = services_.state() == ManagerState::stopping ? ApplicationState::stopping
                                                             : ApplicationState::failed;
        return started.error();
    }
    if (!context_.logger().write(LogLevel::info, LogEvent::startup,
                                 {{LogKey::role, role_name(context_.role()), DataClass::public_metadata},
                                  {LogKey::checkpoint, checkpoint(), DataClass::public_metadata}})) {
        startup_failed_ = true;
        (void)services_.shutdown();
        state_ = services_.state() == ManagerState::stopping ? ApplicationState::stopping
                                                             : ApplicationState::failed;
        return Error{ErrorCode::internal, "Logging output failed."};
    }
    logged_start_ = true;
    state_ = ApplicationState::running;
    return {};
}
Result<void> Application::shutdown() {
    return shutdown(options_.shutdown_timeout);
}
Result<void> Application::shutdown(std::chrono::milliseconds timeout) {
    if (state_ == ApplicationState::starting)
        return Error{ErrorCode::internal, "Cannot shut down from a startup hook.",
                     ErrorReason::invalid_state};
    auto stopped = services_.shutdown(timeout);
    if (!stopped && (stopped.error().reason == ErrorReason::invalid_budget ||
                     stopped.error().reason == ErrorReason::invalid_state))
        return stopped;
    if (!services_.shutdown_report().complete) {
        if (services_.state() == ManagerState::stopping)
            state_ = ApplicationState::stopping;
        return stopped;
    }
    state_ = startup_failed_ || !stopped ? ApplicationState::failed : ApplicationState::stopped;
    if (logged_start_ && !logged_stop_) {
        logged_stop_ = true;
        if (!context_.logger().write(
                LogLevel::info, LogEvent::shutdown,
                {{LogKey::role, role_name(context_.role()), DataClass::public_metadata}}) ||
            !context_.logger().flush())
            logging_error_ = Error{ErrorCode::internal, "Logging output failed during shutdown."};
    }
    if (logging_error_) {
        state_ = ApplicationState::failed;
        return *logging_error_;
    }
    return stopped;
}
void Application::request_stop() noexcept {
    services_.request_stop();
}
std::stop_token Application::stop_token() const noexcept {
    return stop_.get_token();
}
ApplicationState Application::state() const noexcept {
    return state_;
}
} // namespace ghostwire::core
