#pragma once
#include "ghostwire/core/result.hpp"
#include <cstddef>
#include <filesystem>
#include <string_view>

namespace ghostwire::platform {
struct UserDirectories {
    std::filesystem::path data, config, cache;
};
core::Result<UserDirectories> user_directories();
// CLI/config text is UTF-8; filesystem operations keep native path values.
// path.string() is NOT the UTF-8 boundary on Windows. Keep invalid input rejected.
core::Result<std::filesystem::path> path_from_utf8(std::string_view text);
std::string path_to_utf8(const std::filesystem::path &path);
core::Result<std::string> read_text_file(const std::filesystem::path &path, std::size_t max_bytes);
core::Result<void> ensure_directory(const std::filesystem::path &path);
} // namespace ghostwire::platform
