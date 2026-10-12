#pragma once
#include <cstddef>

namespace ghostwire::core {
struct ResourceLimits {
    std::size_t worker_limit = 2;
    std::size_t queue_limit = 1024;
};
inline constexpr std::size_t max_config_bytes = 64 * 1024;
inline constexpr std::size_t max_argument_bytes = 4096;
// Logical budgets: reservations account declared work, not total process RSS.
struct ResourcePolicy {
    std::size_t memory_budget_bytes = 64 * 1024 * 1024;
    unsigned cpu_headroom_percent = 20;
};
struct ResourceBudget {
    std::size_t active_limit = 1;
    std::size_t queued_limit = 1024;
    std::size_t memory_bytes = 64 * 1024 * 1024;
};
} // namespace ghostwire::core
