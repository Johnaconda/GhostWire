#pragma once
#include <string_view>

namespace ghostwire::platform {
struct PlatformInfo {
    std::string_view operating_system;
    std::string_view architecture;
    unsigned logical_processors;
};
PlatformInfo platform_info() noexcept;
} // namespace ghostwire::platform
