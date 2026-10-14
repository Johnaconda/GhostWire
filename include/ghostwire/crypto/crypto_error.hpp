#pragma once
#include "ghostwire/core/result.hpp"
#include <string_view>

namespace ghostwire::crypto {
enum class ErrorCode {
    invalid_limits,
    invalid_size,
    invalid_key,
    cancelled,
    busy,
    resource_exhausted,
    allocation_failed,
    entropy_unavailable,
    initialization_failed,
    authentication_failed,
    invalid_signature,
    provider_failure
};
enum class Operation { initialize, random, import_key, generate_key, encrypt, decrypt, sign, verify, hash };
struct Error {
    ErrorCode code;
    Operation operation;
    bool operator==(const Error &) const = default;
};
template <typename T> using Result = core::Result<T, Error>;
// Fixed metadata only. Never include provider exceptions, input or key IDs.
[[nodiscard]] constexpr std::string_view describe(ErrorCode code) noexcept {
    switch (code) {
    case ErrorCode::invalid_limits:
        return "Invalid cryptographic resource limits.";
    case ErrorCode::invalid_size:
        return "Invalid cryptographic input size.";
    case ErrorCode::invalid_key:
        return "Invalid or empty key handle.";
    case ErrorCode::cancelled:
        return "Cryptographic operation cancelled.";
    case ErrorCode::busy:
        return "Cryptographic provider is busy.";
    case ErrorCode::resource_exhausted:
        return "Cryptographic resource budget exhausted.";
    case ErrorCode::allocation_failed:
        return "Cryptographic allocation failed.";
    case ErrorCode::entropy_unavailable:
        return "Cryptographic entropy unavailable.";
    case ErrorCode::initialization_failed:
        return "Cryptographic provider initialization failed.";
    case ErrorCode::authentication_failed:
        return "Ciphertext authentication failed.";
    case ErrorCode::invalid_signature:
        return "Signature verification failed.";
    case ErrorCode::provider_failure:
        return "Cryptographic provider operation failed.";
    }
    return "Unknown cryptographic error.";
}
} // namespace ghostwire::crypto
