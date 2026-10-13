#pragma once

#include <cstdint>
#include <string>

namespace ghostwire::core {
enum class ErrorCode { usage = 2, configuration = 3, filesystem = 4, internal = 70 };
// Reasons refine errors without changing the established process exit codes.
enum class ErrorReason {
    none,
    invalid_state,
    invalid_registration,
    duplicate_service,
    missing_dependency,
    dependency_cycle,
    cancelled,
    deadline_exceeded,
    service_start_failed,
    service_stop_failed,
    service_exception,
    dependency_busy,
    resource_exhausted,
    admission_closed,
    invalid_budget,
    outstanding_work,
    invalid_process_options,
    process_launch_failed,
    process_control_failed,
    process_wait_failed,
    output_read_failed,
    unsupported,
    unavailable,
    native_failure
};

// Messages must be developer-authored, contain no input values, paths or secrets.
struct Error {
    ErrorCode code;
    std::string message;
    ErrorReason reason = ErrorReason::none;
    std::uint32_t subject = 0; // Local service ID, never a peer/user identifier.
    std::uint32_t dependency = 0;
};
constexpr int exit_code(ErrorCode code) noexcept {
    return static_cast<int>(code);
}
} // namespace ghostwire::core
