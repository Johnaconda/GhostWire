#pragma once
#include "ghostwire/core/service.hpp"
#include <array>
#include <memory>
#include <optional>
#include <span>

namespace ghostwire::core {
inline constexpr std::size_t max_services = 64;
inline constexpr std::size_t max_service_dependencies = 16;
enum class ServiceState { registered, starting, running, stopping, stopped };
enum class ManagerState { created, starting, running, stopping, stopped, failed };
enum class ServicePhase { validation, startup, shutdown };
struct ServiceFailure {
    ServiceId service;
    ServicePhase phase = ServicePhase::validation;
    ErrorCode code = ErrorCode::internal;
    ErrorReason reason = ErrorReason::none;
    ServiceId dependency;
};
struct ShutdownReport {
    std::array<ServiceFailure, max_services + 1> failures{};
    std::size_t failure_count = 0;
    bool complete = false;
};
struct ServiceDiagnostic {
    ServiceId id;
    ServiceState state = ServiceState::registered;
    ErrorReason last_failure = ErrorReason::none;
    bool cleanup_pending = false;
};
struct ServiceSnapshot {
    ManagerState state = ManagerState::created;
    std::array<ServiceDiagnostic, max_services> services{};
    std::size_t count = 0;
    bool stop_requested = false;
    AdmissionSnapshot admission;
};
struct ServiceOrder {
    std::array<ServiceId, max_services> services{};
    std::size_t count = 0;
};

// Explicit owner, not a service locator. All methods except request_stop() are
// owner-thread only and hooks must not reenter lifecycle/registration methods.
// Context and stop source must outlive this object and refer to the same stop state.
class ServiceManager {
  public:
    ServiceManager(const ApplicationContext &context, std::stop_source &stop,
                   std::chrono::milliseconds shutdown_timeout = default_shutdown_timeout);
    ~ServiceManager() noexcept;
    ServiceManager(const ServiceManager &) = delete;
    ServiceManager &operator=(const ServiceManager &) = delete;
    // Consumes ownership on success OR rejection. Dependents receive explicit
    // references at composition time; IDs only establish lifecycle ordering.
    Result<void> add(ServiceId id, std::unique_ptr<Service> service,
                     std::span<const ServiceId> dependencies = {});
    [[nodiscard]] Result<ServiceOrder> initialization_order() const;
    Result<void> start();
    Result<void> shutdown();
    Result<void> shutdown(std::chrono::milliseconds timeout);
    void request_stop() noexcept;
    [[nodiscard]] ManagerState state() const noexcept { return state_; }
    [[nodiscard]] ServiceSnapshot snapshot() const;
    [[nodiscard]] const ShutdownReport &shutdown_report() const noexcept { return report_; }
    [[nodiscard]] const std::optional<Error> &startup_error() const noexcept { return startup_error_; }

  private:
    struct Entry {
        std::unique_ptr<Service> service;
        ServiceDiagnostic diagnostic;
        std::array<ServiceId, max_service_dependencies> dependencies{};
        std::size_t dependency_count = 0;
    };
    [[nodiscard]] std::size_t find(ServiceId id) const noexcept;
    [[nodiscard]] Result<ServiceOrder> plan() const;
    [[nodiscard]] bool pending() const noexcept;
    Result<void> stop_services(Deadline deadline);
    Result<void> report_result() const;
    Result<void> fail_start(Error error);
    void record(ServiceFailure failure) noexcept;
    const ApplicationContext &context_;
    std::stop_source &stop_;
    std::chrono::milliseconds shutdown_timeout_;
    std::array<Entry, max_services> entries_{};
    std::size_t count_ = 0;
    ServiceOrder order_{};
    ManagerState state_ = ManagerState::created;
    bool operation_active_ = false;
    std::optional<Error> startup_error_;
    ShutdownReport report_{};
};
} // namespace ghostwire::core
