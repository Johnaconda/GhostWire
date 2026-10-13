#pragma once
#include "ghostwire/core/result.hpp"
#include <stop_token>

namespace ghostwire::platform {
enum class SecureStorageProvider { none, windows_dpapi, macos_keychain };
enum class CapabilityAvailability { available, unavailable, unsupported };
enum class SecureStorageProbe { native_api, disabled };
enum class SecureStorageReason { api_present, disabled_by_policy, adapter_not_built, api_missing };
struct SecureStorageCapability {
    SecureStorageProvider provider = SecureStorageProvider::none;
    bool supported = false;
    CapabilityAvailability api_availability = CapabilityAvailability::unsupported;
    SecureStorageReason reason = SecureStorageReason::adapter_not_built;
    // A02 only discovers native API entry points. No account/keychain unlock,
    // secret operation or persistence is tested or permitted by this result.
    bool persistence_ready = false;
};
// Local read-only API discovery, never a key store or a claim of A11-PRE readiness.
// Native loader failure is a Result error, distinct from a missing facility.
core::Result<SecureStorageCapability>
probe_secure_storage(SecureStorageProbe policy = SecureStorageProbe::native_api, std::stop_token stop = {});
} // namespace ghostwire::platform
