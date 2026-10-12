#pragma once
#include <string_view>

namespace ghostwire::core {
enum class Role { desktop, server, relay, cli };
constexpr std::string_view role_name(Role role) noexcept {
    switch (role) {
    case Role::desktop:
        return "desktop";
    case Role::server:
        return "server";
    case Role::relay:
        return "relay";
    case Role::cli:
        return "cli";
    }
    return "unknown";
}
} // namespace ghostwire::core
