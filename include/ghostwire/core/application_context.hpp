#pragma once
#include "ghostwire/core/clock.hpp"
#include "ghostwire/core/config.hpp"
#include "ghostwire/core/paths.hpp"
#include "ghostwire/core/resource_admission.hpp"
#include <utility>

namespace ghostwire::core {
// Owned by Application, destroyed after all registered services. Config/paths are
// immutable owned snapshots. Referenced facilities must outlive this context.
// No registry lookup: domain dependencies are injected into service constructors.
class ApplicationContext {
  public:
    ApplicationContext(Role role, Config config, Paths paths, Logger &logger, const Clock &clock,
                       ResourceAdmission &admission, std::stop_token stop)
        : role_(role), config_(std::move(config)), paths_(std::move(paths)), logger_(logger), clock_(clock),
          admission_(admission), stop_(std::move(stop)) {}
    ApplicationContext(const ApplicationContext &) = delete;
    ApplicationContext &operator=(const ApplicationContext &) = delete;
    [[nodiscard]] Role role() const noexcept { return role_; }
    [[nodiscard]] const Config &config() const noexcept { return config_; }
    [[nodiscard]] const Paths &paths() const noexcept { return paths_; }
    [[nodiscard]] Logger &logger() const noexcept { return logger_; }
    [[nodiscard]] const Clock &clock() const noexcept { return clock_; }
    [[nodiscard]] ResourceAdmission &admission() const noexcept { return admission_; }
    [[nodiscard]] std::stop_token stop_token() const noexcept { return stop_; }

  private:
    const Role role_;
    const Config config_;
    const Paths paths_;
    Logger &logger_;
    const Clock &clock_;
    ResourceAdmission &admission_;
    const std::stop_token stop_;
};
} // namespace ghostwire::core
