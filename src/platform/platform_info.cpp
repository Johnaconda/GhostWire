#include "ghostwire/platform/platform_info.hpp"
#include <algorithm>
#include <thread>

namespace ghostwire::platform {
PlatformInfo platform_info() noexcept {
#if defined(_WIN32)
    constexpr auto os = "Windows";
#elif defined(__APPLE__)
    constexpr auto os = "macOS";
#elif defined(__linux__)
    constexpr auto os = "Linux";
#else
    constexpr auto os = "Unknown";
#endif
#if defined(_M_X64) || defined(__x86_64__)
    constexpr auto arch = "x86_64";
#elif defined(_M_ARM64) || defined(__aarch64__)
    constexpr auto arch = "arm64";
#else
    constexpr auto arch = "other";
#endif
    return {os, arch, std::max(1U, std::thread::hardware_concurrency())};
}
} // namespace ghostwire::platform
