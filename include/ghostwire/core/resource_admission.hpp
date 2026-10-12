#pragma once
#include "ghostwire/core/resource_limits.hpp"
#include "ghostwire/core/result.hpp"
#include <memory>
#include <stop_token>

namespace ghostwire::core {
// Invalid policy produces zero capacity, rejected when admission is opened.
ResourceBudget resource_budget(ResourceLimits limits, unsigned logical_processors,
                               ResourcePolicy policy = {}) noexcept;
enum class WorkKind { queued, active };
struct AdmissionSnapshot {
    ResourceBudget budget;
    std::size_t active = 0, queued = 0, reserved_bytes = 0;
    std::size_t accepted = 0, rejected = 0; // Saturating counters.
    bool accepting = false;
};
struct AdmissionState;

// Move-only reservation. It owns accounting state, not payload or an executor.
// Distinct leases may be used on different threads; one lease needs exclusive ownership.
// Releasing after the admission owner is destroyed is safe and never reopens admission.
class WorkLease {
  public:
    WorkLease() = default;
    ~WorkLease();
    WorkLease(WorkLease &&other) noexcept;
    WorkLease &operator=(WorkLease &&other) noexcept;
    WorkLease(const WorkLease &) = delete;
    WorkLease &operator=(const WorkLease &) = delete;
    Result<void> try_activate(std::stop_token stop = {});
    void release() noexcept;
    [[nodiscard]] explicit operator bool() const noexcept { return bool(state_); }

  private:
    friend class ResourceAdmission;
    WorkLease(std::shared_ptr<AdmissionState> state, WorkKind kind, std::size_t bytes);
    std::shared_ptr<AdmissionState> state_;
    WorkKind kind_ = WorkKind::queued;
    std::size_t bytes_ = 0;
};

// Thread-safe, nonblocking backpressure. No queue, allocation of payloads or threads.
// open() is one-shot; close() is terminal, including when called before open().
class ResourceAdmission {
  public:
    explicit ResourceAdmission(ResourceBudget budget);
    ~ResourceAdmission();
    ResourceAdmission(const ResourceAdmission &) = delete;
    ResourceAdmission &operator=(const ResourceAdmission &) = delete;
    Result<void> open();
    void close() noexcept;
    Result<WorkLease> try_acquire(WorkKind kind, std::size_t bytes, std::stop_token stop = {});
    [[nodiscard]] AdmissionSnapshot snapshot() const;

  private:
    std::shared_ptr<AdmissionState> state_;
};
} // namespace ghostwire::core
