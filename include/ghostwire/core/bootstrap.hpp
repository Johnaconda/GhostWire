#pragma once
#include "ghostwire/core/role.hpp"
#include <ostream>
#include <span>
#include <string_view>

namespace ghostwire::core {
// args excludes executable name. Parse/config errors never echo supplied values.
int run_cli(Role role, std::span<const std::string_view> args, std::ostream &out, std::ostream &err);
int bootstrap_main(Role role, int argc, char **argv) noexcept;
} // namespace ghostwire::core
