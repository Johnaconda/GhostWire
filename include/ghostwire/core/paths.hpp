#pragma once
#include "ghostwire/core/role.hpp"
#include "ghostwire/platform/filesystem.hpp"
#include <filesystem>

namespace ghostwire::core {
struct Paths {
    std::filesystem::path data, config, cache, logs, runtime;
};
// Pure lexical resolution; no directories created. Roles have isolated roots.
Result<Paths> resolve_paths(Role role, const std::filesystem::path &override_root,
                            const platform::UserDirectories &defaults);
Result<void> initialize_paths(const Paths &paths);
} // namespace ghostwire::core
