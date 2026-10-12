#include "ghostwire/core/paths.hpp"

namespace ghostwire::core {
Result<Paths> resolve_paths(Role role, const std::filesystem::path &root,
                            const platform::UserDirectories &defaults) {
    if (role_name(role) == "unknown")
        return Error{ErrorCode::internal, "Invalid application role."};
    const auto leaf = std::filesystem::path(role_name(role));
    Paths paths;
    if (!root.empty()) {
        if (!root.is_absolute())
            return Error{ErrorCode::configuration, "data_root must be an absolute path."};
        const auto role_root = root.lexically_normal() / leaf;
        paths.data = role_root / "data";
        paths.config = role_root / "config";
        paths.cache = role_root / "cache";
    } else {
        if (!defaults.data.is_absolute() || !defaults.config.is_absolute() || !defaults.cache.is_absolute())
            return Error{ErrorCode::filesystem, "User directories must be absolute."};
        paths.data = defaults.data / "GhostWire" / leaf;
        paths.config = defaults.config / "GhostWire" / leaf;
        paths.cache = defaults.cache / "GhostWire" / leaf;
    }
    paths.data = paths.data.lexically_normal();
    paths.config = paths.config.lexically_normal();
    paths.cache = paths.cache.lexically_normal();
    paths.logs = paths.data / "logs";
    paths.runtime = paths.cache / "runtime";
    return paths;
}
Result<void> initialize_paths(const Paths &paths) {
    for (const auto *path : {&paths.data, &paths.config, &paths.cache, &paths.logs, &paths.runtime}) {
        auto created = platform::ensure_directory(*path);
        if (!created)
            return created.error();
    }
    return {};
}
} // namespace ghostwire::core
