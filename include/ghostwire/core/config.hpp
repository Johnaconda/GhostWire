#pragma once
#include "ghostwire/core/logger.hpp"
#include "ghostwire/core/resource_limits.hpp"
#include "ghostwire/core/result.hpp"
#include <filesystem>
#include <string_view>

namespace ghostwire::core {
enum class PrivacyMode { direct, protected_mode, privacy_first };
struct Config {
    unsigned schema_version = 1;
    LogLevel log_level = LogLevel::info;
    PrivacyMode privacy_mode = PrivacyMode::protected_mode;
    ResourceLimits limits;
    std::filesystem::path data_root; // Empty uses OS defaults; otherwise absolute.
};
Result<Config> parse_config(std::string_view text);
Result<Config> load_config(const std::filesystem::path &path);
Result<void> validate_config(const Config &config);
} // namespace ghostwire::core
