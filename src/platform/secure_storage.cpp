#include "ghostwire/platform/secure_storage.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif defined(__APPLE__)
#include <dlfcn.h>
#endif

namespace ghostwire::platform {
core::Result<SecureStorageCapability> probe_secure_storage(SecureStorageProbe policy, std::stop_token stop) {
    using core::Error;
    using core::ErrorCode;
    using core::ErrorReason;
    if (stop.stop_requested())
        return Error{ErrorCode::internal, "Secure storage probe cancelled.", ErrorReason::cancelled};
    if (policy != SecureStorageProbe::native_api && policy != SecureStorageProbe::disabled)
        return Error{ErrorCode::usage, "Invalid secure storage probe policy.", ErrorReason::invalid_state};
    SecureStorageCapability result;
#ifdef _WIN32
    result.provider = SecureStorageProvider::windows_dpapi;
    result.supported = true;
#elif defined(__APPLE__)
    result.provider = SecureStorageProvider::macos_keychain;
    result.supported = true;
#endif
    if (policy == SecureStorageProbe::disabled) {
        result.api_availability = CapabilityAvailability::unavailable;
        result.reason = SecureStorageReason::disabled_by_policy;
        return result;
    }
#ifdef _WIN32
    const auto module = LoadLibraryExW(L"crypt32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) {
        const auto error = GetLastError();
        if (error == ERROR_MOD_NOT_FOUND || error == ERROR_DLL_NOT_FOUND) {
            result.api_availability = CapabilityAvailability::unavailable;
            result.reason = SecureStorageReason::api_missing;
            return result;
        }
        return Error{ErrorCode::internal, "Cannot inspect native secure storage API.",
                     ErrorReason::native_failure};
    }
    const bool present =
        GetProcAddress(module, "CryptProtectData") && GetProcAddress(module, "CryptUnprotectData");
    FreeLibrary(module);
    result.api_availability =
        present ? CapabilityAvailability::available : CapabilityAvailability::unavailable;
    result.reason = present ? SecureStorageReason::api_present : SecureStorageReason::api_missing;
#elif defined(__APPLE__)
    const auto module =
        dlopen("/System/Library/Frameworks/Security.framework/Security", RTLD_LOCAL | RTLD_LAZY);
    if (!module)
        return Error{ErrorCode::internal, "Cannot inspect native secure storage API.",
                     ErrorReason::native_failure};
    const bool present = dlsym(module, "SecItemCopyMatching") && dlsym(module, "SecItemAdd") &&
                         dlsym(module, "SecItemUpdate") && dlsym(module, "SecItemDelete");
    dlclose(module);
    result.api_availability =
        present ? CapabilityAvailability::available : CapabilityAvailability::unavailable;
    result.reason = present ? SecureStorageReason::api_present : SecureStorageReason::api_missing;
#endif
    if (stop.stop_requested())
        return Error{ErrorCode::internal, "Secure storage probe cancelled.", ErrorReason::cancelled};
    return result;
}
} // namespace ghostwire::platform
