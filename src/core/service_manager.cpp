#include "ghostwire/core/service_manager.hpp"
#include <algorithm>
#include <utility>

namespace ghostwire::core {
namespace {
Error invalid_state() {
    return {ErrorCode::internal, "Invalid service lifecycle transition.", ErrorReason::invalid_state};
}
struct OperationGuard {
    explicit OperationGuard(bool &active) : active_(active) { active_ = true; }
    ~OperationGuard() { active_ = false; }
    bool &active_;
};
ErrorCode safe_code(ErrorCode code) noexcept {
    switch (code) {
    case ErrorCode::usage:
    case ErrorCode::configuration:
    case ErrorCode::filesystem:
    case ErrorCode::internal:
        return code;
    }
    return ErrorCode::internal;
}
} // namespace
ServiceManager::ServiceManager(const ApplicationContext &context, std::stop_source &stop,
                               std::chrono::milliseconds shutdown_timeout)
    : context_(context), stop_(stop), shutdown_timeout_(shutdown_timeout) {}
ServiceManager::~ServiceManager() noexcept {
    try {
        (void)shutdown();
    } catch (...) {
        request_stop();
    }
    // Destruct dependents BEFORE dependencies even if they were never started.
    // Destructors must own/join residual work; an uncooperative destructor cannot
    // be safely preempted in-process. Explicit shutdown is the reportable path.
    if (order_.count == 0) {
        try {
            auto planned = plan();
            if (planned)
                order_ = planned.value();
        } catch (...) {
        }
    }
    for (std::size_t i = order_.count; i > 0; --i)
        entries_[find(order_.services[i - 1])].service.reset();
    for (std::size_t i = count_; i > 0; --i)
        entries_[i - 1].service.reset();
}
std::size_t ServiceManager::find(ServiceId id) const noexcept {
    for (std::size_t i = 0; i < count_; ++i)
        if (entries_[i].diagnostic.id == id)
            return i;
    return count_;
}
Result<void> ServiceManager::add(ServiceId id, std::unique_ptr<Service> service,
                                 std::span<const ServiceId> dependencies) {
    if (state_ != ManagerState::created || operation_active_)
        return invalid_state();
    if (!service || id.value == 0 || count_ == max_services || dependencies.size() > max_service_dependencies)
        return Error{ErrorCode::internal, "Invalid or over-budget service registration.",
                     ErrorReason::invalid_registration, id.value};
    if (find(id) != count_)
        return Error{ErrorCode::internal, "Service ID already registered.", ErrorReason::duplicate_service,
                     id.value};
    for (std::size_t i = 0; i < dependencies.size(); ++i) {
        if (dependencies[i].value == 0 ||
            std::find(dependencies.begin(), dependencies.begin() + static_cast<std::ptrdiff_t>(i),
                      dependencies[i]) != dependencies.begin() + static_cast<std::ptrdiff_t>(i))
            return Error{ErrorCode::internal, "Invalid dependency declaration.",
                         ErrorReason::invalid_registration, id.value, dependencies[i].value};
    }
    auto &entry = entries_[count_++];
    entry.service = std::move(service);
    entry.diagnostic.id = id;
    std::copy(dependencies.begin(), dependencies.end(), entry.dependencies.begin());
    entry.dependency_count = dependencies.size();
    return {};
}
Result<ServiceOrder> ServiceManager::plan() const {
    for (std::size_t i = 0; i < count_; ++i)
        for (std::size_t d = 0; d < entries_[i].dependency_count; ++d)
            if (find(entries_[i].dependencies[d]) == count_)
                return Error{ErrorCode::internal, "A required service is missing.",
                             ErrorReason::missing_dependency, entries_[i].diagnostic.id.value,
                             entries_[i].dependencies[d].value};
    ServiceOrder result;
    std::array<bool, max_services> selected{};
    while (result.count < count_) {
        std::size_t next = count_;
        for (std::size_t i = 0; i < count_; ++i) {
            if (selected[i])
                continue;
            bool ready = true;
            for (std::size_t d = 0; d < entries_[i].dependency_count; ++d)
                if (!selected[find(entries_[i].dependencies[d])])
                    ready = false;
            if (ready &&
                (next == count_ || entries_[i].diagnostic.id.value < entries_[next].diagnostic.id.value))
                next = i;
        }
        if (next == count_)
            return Error{ErrorCode::internal, "Service dependencies contain a cycle.",
                         ErrorReason::dependency_cycle};
        selected[next] = true;
        result.services[result.count++] = entries_[next].diagnostic.id;
    }
    return result;
}
Result<ServiceOrder> ServiceManager::initialization_order() const {
    if (state_ != ManagerState::created || operation_active_)
        return invalid_state();
    return plan();
}
bool ServiceManager::pending() const noexcept {
    for (std::size_t i = 0; i < count_; ++i)
        if (entries_[i].diagnostic.cleanup_pending)
            return true;
    return false;
}
void ServiceManager::request_stop() noexcept {
    // No manager state/graph access; callable concurrently. Stop callbacks run
    // synchronously and therefore must be noexcept, short, and nonblocking.
    context_.admission().close();
    (void)stop_.request_stop();
}
Result<void> ServiceManager::fail_start(Error error) {
    startup_error_ = std::move(error);
    auto deadline = Deadline::after(context_.clock(), shutdown_timeout_);
    if (deadline)
        (void)stop_services(deadline.value());
    else {
        request_stop();
        state_ = ManagerState::failed;
    }
    return *startup_error_;
}
Result<void> ServiceManager::start() {
    if (state_ != ManagerState::created || operation_active_)
        return invalid_state();
    OperationGuard guard(operation_active_);
    state_ = ManagerState::starting;
    auto deadline = Deadline::after(context_.clock(), shutdown_timeout_);
    if (!deadline)
        return fail_start(deadline.error());
    if (context_.stop_token() != stop_.get_token())
        return fail_start({ErrorCode::internal, "Context and manager cancellation states differ.",
                           ErrorReason::invalid_registration});
    auto planned = plan();
    if (!planned)
        return fail_start(planned.error());
    order_ = planned.value();
    if (stop_.stop_requested())
        return fail_start({ErrorCode::internal, "Startup cancelled.", ErrorReason::cancelled});
    auto opened = context_.admission().open();
    if (!opened)
        return fail_start(opened.error());
    for (std::size_t i = 0; i < order_.count; ++i) {
        if (stop_.stop_requested())
            return fail_start({ErrorCode::internal, "Startup cancelled.", ErrorReason::cancelled});
        auto &entry = entries_[find(order_.services[i])];
        auto &diagnostic = entry.diagnostic;
        diagnostic.state = ServiceState::starting;
        diagnostic.cleanup_pending = true; // Includes a partially initialized/throwing service.
        std::optional<Error> failure;
        try {
            auto started = entry.service->start(context_);
            if (!started) {
                diagnostic.last_failure = ErrorReason::service_start_failed;
                failure = Error{safe_code(started.error().code), "Service initialization failed.",
                                ErrorReason::service_start_failed, diagnostic.id.value};
            }
        } catch (...) {
            diagnostic.last_failure = ErrorReason::service_exception;
            failure = Error{ErrorCode::internal, "Service initialization raised an exception.",
                            ErrorReason::service_exception, diagnostic.id.value};
        }
        if (failure)
            return fail_start(std::move(*failure));
        diagnostic.state = ServiceState::running;
    }
    if (stop_.stop_requested())
        return fail_start({ErrorCode::internal, "Startup cancelled.", ErrorReason::cancelled});
    state_ = ManagerState::running;
    return {};
}
void ServiceManager::record(ServiceFailure failure) noexcept {
    if (report_.failure_count < report_.failures.size())
        report_.failures[report_.failure_count++] = failure;
}
Result<void> ServiceManager::report_result() const {
    if (report_.failure_count == 0)
        return {};
    const auto &failure = report_.failures[0];
    return Error{failure.code, "Service shutdown did not complete successfully.", failure.reason,
                 failure.service.value, failure.dependency.value};
}
Result<void> ServiceManager::stop_services(Deadline deadline) {
    state_ = ManagerState::stopping;
    report_ = {};
    request_stop();
    const ShutdownContext shutdown{context_.clock(), deadline, stop_.get_token()};
    for (std::size_t n = order_.count; n > 0; --n) {
        auto &entry = entries_[find(order_.services[n - 1])];
        auto &diagnostic = entry.diagnostic;
        if (!diagnostic.cleanup_pending)
            continue;
        ServiceId blocker;
        // A dependent that still owns work keeps its dependencies alive. Other
        // independent branches may still finish, in reverse deterministic order.
        for (std::size_t i = 0; i < count_; ++i) {
            if (!entries_[i].diagnostic.cleanup_pending)
                continue;
            for (std::size_t d = 0; d < entries_[i].dependency_count; ++d)
                if (entries_[i].dependencies[d] == diagnostic.id)
                    blocker = entries_[i].diagnostic.id;
        }
        if (blocker.value != 0) {
            record({diagnostic.id, ServicePhase::shutdown, ErrorCode::internal, ErrorReason::dependency_busy,
                    blocker});
            continue;
        }
        diagnostic.state = ServiceState::stopping;
        ErrorReason failure = ErrorReason::none;
        ErrorCode code = ErrorCode::internal;
        try {
            auto stopped = entry.service->stop(shutdown);
            if (stopped) {
                diagnostic.cleanup_pending = false;
                diagnostic.state = ServiceState::stopped;
            } else {
                failure = stopped.error().reason == ErrorReason::deadline_exceeded
                              ? ErrorReason::deadline_exceeded
                              : ErrorReason::service_stop_failed;
                code = safe_code(stopped.error().code);
            }
        } catch (...) {
            failure = ErrorReason::service_exception;
        }
        if (failure == ErrorReason::none && context_.clock().now() > deadline.time_point())
            failure = ErrorReason::deadline_exceeded;
        if (failure != ErrorReason::none) {
            diagnostic.last_failure = failure;
            record({diagnostic.id, ServicePhase::shutdown, code, failure, {}});
        }
    }
    const auto admission = context_.admission().snapshot();
    if (admission.active != 0 || admission.queued != 0)
        record({{}, ServicePhase::shutdown, ErrorCode::internal, ErrorReason::outstanding_work, {}});
    // Also detect time consumed by synchronous stop callbacks or an empty graph.
    if (report_.failure_count == 0 && context_.clock().now() > deadline.time_point())
        record({{}, ServicePhase::shutdown, ErrorCode::internal, ErrorReason::deadline_exceeded, {}});
    report_.complete = !pending() && admission.active == 0 && admission.queued == 0;
    if (report_.complete)
        state_ = startup_error_ || report_.failure_count != 0 ? ManagerState::failed : ManagerState::stopped;
    return report_result();
}
Result<void> ServiceManager::shutdown() {
    return shutdown(shutdown_timeout_);
}
Result<void> ServiceManager::shutdown(std::chrono::milliseconds timeout) {
    if (operation_active_)
        return invalid_state();
    auto deadline = Deadline::after(context_.clock(), timeout);
    if (!deadline)
        return deadline.error(); // Invalid request has no side effects.
    if (report_.complete)
        return report_result();
    OperationGuard guard(operation_active_);
    return stop_services(deadline.value());
}
ServiceSnapshot ServiceManager::snapshot() const {
    ServiceSnapshot result;
    result.state = state_;
    result.count = count_;
    result.stop_requested = stop_.stop_requested();
    result.admission = context_.admission().snapshot();
    for (std::size_t i = 0; i < count_; ++i)
        result.services[i] = entries_[i].diagnostic;
    std::sort(result.services.begin(), result.services.begin() + static_cast<std::ptrdiff_t>(count_),
              [](const auto &a, const auto &b) { return a.id.value < b.id.value; });
    return result;
}
} // namespace ghostwire::core
