#include "ghostwire/core/resource_admission.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <mutex>
#include <utility>

namespace ghostwire::core {
struct AdmissionState {
    explicit AdmissionState(ResourceBudget budget) { snapshot.budget = budget; }
    std::mutex mutex;
    AdmissionSnapshot snapshot;
    bool opened = false, closed = false;
};
namespace {
void increment(std::size_t &counter) noexcept {
    if (counter != std::numeric_limits<std::size_t>::max())
        ++counter;
}
Error rejected(AdmissionState &state, ErrorReason reason) {
    increment(state.snapshot.rejected);
    return {ErrorCode::internal, "Work admission rejected.", reason};
}
} // namespace
ResourceBudget resource_budget(ResourceLimits limits, unsigned logical_processors,
                               ResourcePolicy policy) noexcept {
    if (limits.worker_limit < 1 || limits.worker_limit > 64 || limits.queue_limit < 16 ||
        limits.queue_limit > 65536 || policy.cpu_headroom_percent > 99 || policy.memory_budget_bytes == 0)
        return {0, 0, 0};
    const auto cpus = std::max<std::uint64_t>(1, logical_processors);
    const auto reserved = (cpus * policy.cpu_headroom_percent + 99) / 100;
    const auto usable = std::max<std::uint64_t>(1, cpus - reserved);
    return {static_cast<std::size_t>(std::min<std::uint64_t>(limits.worker_limit, usable)),
            limits.queue_limit, policy.memory_budget_bytes};
}
ResourceAdmission::ResourceAdmission(ResourceBudget budget)
    : state_(std::make_shared<AdmissionState>(budget)) {}
ResourceAdmission::~ResourceAdmission() {
    close();
}
Result<void> ResourceAdmission::open() {
    std::lock_guard lock(state_->mutex);
    if (state_->opened || state_->closed)
        return Error{ErrorCode::internal, "Admission cannot be reopened.", ErrorReason::invalid_state};
    const auto &budget = state_->snapshot.budget;
    if (budget.active_limit == 0 || budget.active_limit > 64 || budget.queued_limit == 0 ||
        budget.queued_limit > 65536 || budget.memory_bytes == 0)
        return Error{ErrorCode::configuration, "Invalid resource budget.", ErrorReason::invalid_budget};
    state_->opened = true;
    state_->snapshot.accepting = true;
    return {};
}
void ResourceAdmission::close() noexcept {
    std::lock_guard lock(state_->mutex);
    state_->closed = true;
    state_->snapshot.accepting = false;
}
Result<WorkLease> ResourceAdmission::try_acquire(WorkKind kind, std::size_t bytes, std::stop_token stop) {
    std::lock_guard lock(state_->mutex);
    auto &s = state_->snapshot;
    if (stop.stop_requested())
        return rejected(*state_, ErrorReason::cancelled);
    if (!s.accepting)
        return rejected(*state_, ErrorReason::admission_closed);
    if (kind != WorkKind::queued && kind != WorkKind::active)
        return rejected(*state_, ErrorReason::invalid_registration);
    if ((kind == WorkKind::active && s.active >= s.budget.active_limit) ||
        (kind == WorkKind::queued && s.queued >= s.budget.queued_limit) ||
        bytes > s.budget.memory_bytes - s.reserved_bytes)
        return rejected(*state_, ErrorReason::resource_exhausted);
    if (kind == WorkKind::active)
        ++s.active;
    else
        ++s.queued;
    s.reserved_bytes += bytes;
    increment(s.accepted);
    return WorkLease(state_, kind, bytes);
}
AdmissionSnapshot ResourceAdmission::snapshot() const {
    std::lock_guard lock(state_->mutex);
    return state_->snapshot;
}
WorkLease::WorkLease(std::shared_ptr<AdmissionState> state, WorkKind kind, std::size_t bytes)
    : state_(std::move(state)), kind_(kind), bytes_(bytes) {}
WorkLease::~WorkLease() {
    release();
}
WorkLease::WorkLease(WorkLease &&other) noexcept
    : state_(std::move(other.state_)), kind_(other.kind_), bytes_(other.bytes_) {}
WorkLease &WorkLease::operator=(WorkLease &&other) noexcept {
    if (this != &other) {
        release();
        state_ = std::move(other.state_);
        kind_ = other.kind_;
        bytes_ = other.bytes_;
    }
    return *this;
}
Result<void> WorkLease::try_activate(std::stop_token stop) {
    if (!state_)
        return Error{ErrorCode::internal, "Reservation is empty.", ErrorReason::invalid_state};
    std::lock_guard lock(state_->mutex);
    auto &s = state_->snapshot;
    if (stop.stop_requested())
        return rejected(*state_, ErrorReason::cancelled);
    if (!s.accepting)
        return rejected(*state_, ErrorReason::admission_closed);
    if (kind_ != WorkKind::queued)
        return rejected(*state_, ErrorReason::invalid_state);
    if (s.active >= s.budget.active_limit)
        return rejected(*state_, ErrorReason::resource_exhausted);
    --s.queued;
    ++s.active;
    kind_ = WorkKind::active;
    return {};
}
void WorkLease::release() noexcept {
    if (!state_)
        return;
    {
        std::lock_guard lock(state_->mutex);
        auto &s = state_->snapshot;
        if (kind_ == WorkKind::active)
            --s.active;
        else
            --s.queued;
        s.reserved_bytes -= bytes_;
    }
    state_.reset();
}
} // namespace ghostwire::core
