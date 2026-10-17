#pragma once
#include "ghostwire/crypto/crypto_error.hpp"
#include <string_view>

namespace ghostwire::identity {
enum class ErrorCode {
    invalid_argument,
    invalid_state,
    invalid_handle,
    wrong_scope,
    malformed_record,
    unsupported_version,
    unsupported_suite,
    invalid_signature,
    untrusted_authority,
    blocked,
    not_yet_valid,
    expired,
    time_untrusted,
    stale_revision,
    conflicting_revision,
    unauthorized_device,
    revision_exhausted,
    resource_exhausted,
    busy,
    cancelled,
    deadline_exceeded,
    crypto_failure
};
struct Error {
    ErrorCode code;
    bool operator==(const Error &) const = default;
};
template <typename T> using Result = core::Result<T, Error>;
// No source strings, key material, identifiers or provider exception text.
[[nodiscard]] constexpr Error from_crypto_error(crypto::Error error) noexcept {
    switch (error.code) {
    case crypto::ErrorCode::cancelled:
        return {ErrorCode::cancelled};
    case crypto::ErrorCode::busy:
        return {ErrorCode::busy};
    case crypto::ErrorCode::allocation_failed:
    case crypto::ErrorCode::resource_exhausted:
        return {ErrorCode::resource_exhausted};
    case crypto::ErrorCode::invalid_signature:
        return {ErrorCode::invalid_signature};
    default:
        return {ErrorCode::crypto_failure};
    }
}
[[nodiscard]] constexpr std::string_view describe(ErrorCode code) noexcept {
    switch (code) {
    case ErrorCode::invalid_argument:
        return "Invalid identity argument.";
    case ErrorCode::invalid_state:
        return "Identity operation is unavailable in this state.";
    case ErrorCode::invalid_handle:
        return "Invalid or expired identity handle.";
    case ErrorCode::wrong_scope:
        return "Identity scope does not match.";
    case ErrorCode::malformed_record:
        return "Invalid identity input.";
    case ErrorCode::unsupported_version:
        return "Unsupported identity record version.";
    case ErrorCode::unsupported_suite:
        return "Unsupported identity signature suite.";
    case ErrorCode::invalid_signature:
        return "Identity signature verification failed.";
    case ErrorCode::untrusted_authority:
        return "Identity authority is not trusted.";
    case ErrorCode::blocked:
        return "Identity authority is blocked.";
    case ErrorCode::not_yet_valid:
        return "Identity record is not yet valid.";
    case ErrorCode::expired:
        return "Identity record has expired.";
    case ErrorCode::time_untrusted:
        return "Identity verification time is not trusted.";
    case ErrorCode::stale_revision:
        return "Identity record revision is stale.";
    case ErrorCode::conflicting_revision:
        return "Identity record revision conflicts.";
    case ErrorCode::unauthorized_device:
        return "Device is not authorized by the current record.";
    case ErrorCode::revision_exhausted:
        return "Identity revision space is exhausted.";
    case ErrorCode::resource_exhausted:
        return "Identity resource budget is exhausted.";
    case ErrorCode::busy:
        return "Identity operation is already in progress.";
    case ErrorCode::cancelled:
        return "Identity operation was cancelled.";
    case ErrorCode::deadline_exceeded:
        return "Identity operation deadline was exceeded.";
    case ErrorCode::crypto_failure:
        return "Identity cryptographic operation failed.";
    }
    return "Unknown identity error.";
}
} // namespace ghostwire::identity
