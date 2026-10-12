#include "ghostwire/core/config.hpp"
#include "ghostwire/platform/filesystem.hpp"
#include <charconv>
#include <set>

namespace ghostwire::core {
namespace {
std::string_view trim(std::string_view text) {
    const auto first = text.find_first_not_of(" \t\r");
    if (first == text.npos)
        return {};
    return text.substr(first, text.find_last_not_of(" \t\r") - first + 1);
}
Result<std::size_t> number(std::string_view text) {
    std::size_t value{};
    const auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (text.empty() || ec != std::errc{} || end != text.data() + text.size())
        return Error{ErrorCode::configuration, "Expected an unsigned decimal integer."};
    return value;
}
Error invalid(std::size_t line, std::string_view reason) {
    return {ErrorCode::configuration,
            "Configuration line " + std::to_string(line) + ": " + std::string(reason)};
}
} // namespace

Result<void> validate_config(const Config &config) {
    if (config.schema_version != 1)
        return Error{ErrorCode::configuration, "Unsupported configuration schema."};
    if (config.limits.worker_limit < 1 || config.limits.worker_limit > 64)
        return Error{ErrorCode::configuration, "worker_limit must be between 1 and 64."};
    if (config.limits.queue_limit < 16 || config.limits.queue_limit > 65536)
        return Error{ErrorCode::configuration, "queue_limit must be between 16 and 65536."};
    if (!config.data_root.empty() && !config.data_root.is_absolute())
        return Error{ErrorCode::configuration, "data_root must be an absolute path."};
    switch (config.log_level) {
    case LogLevel::debug:
    case LogLevel::info:
    case LogLevel::warning:
    case LogLevel::error:
    case LogLevel::off:
        break;
    default:
        return Error{ErrorCode::configuration, "Invalid log level."};
    }
    switch (config.privacy_mode) {
    case PrivacyMode::direct:
    case PrivacyMode::protected_mode:
    case PrivacyMode::privacy_first:
        break;
    default:
        return Error{ErrorCode::configuration, "Invalid privacy mode."};
    }
    return {};
}

Result<Config> parse_config(std::string_view text) {
    if (text.size() > max_config_bytes)
        return Error{ErrorCode::configuration, "Configuration exceeds 64 KiB."};
    // Permit UTF-8 BOM, CRLF and printable UTF-8 path bytes; reject control bytes.
    if (text.starts_with("\xEF\xBB\xBF"))
        text.remove_prefix(3);
    for (const char code_unit : text) {
        const auto byte = static_cast<unsigned char>(code_unit);
        if ((byte < 32 && byte != '\n' && byte != '\r' && byte != '\t') || byte == 127)
            return Error{ErrorCode::configuration, "Configuration contains a control character."};
    }
    Config config;
    std::set<std::string_view> seen;
    std::size_t line_number = 0;
    while (!text.empty()) {
        ++line_number;
        const auto end = text.find('\n');
        const auto line = trim(text.substr(0, end));
        if (end == text.npos)
            text = {};
        else
            text.remove_prefix(end + 1);
        if (line.empty() || line.front() == '#')
            continue;
        const auto eq = line.find('=');
        if (eq == line.npos)
            return invalid(line_number, "Expected key=value.");
        const auto key = trim(line.substr(0, eq));
        const auto value = trim(line.substr(eq + 1));
        if (key.empty() || value.empty())
            return invalid(line_number, "Empty key or value.");
        if (!seen.insert(key).second)
            return invalid(line_number, "Duplicate key.");
        if (key == "schema_version" || key == "worker_limit" || key == "queue_limit") {
            const auto parsed = number(value);
            if (!parsed)
                return invalid(line_number, "Expected an unsigned decimal integer.");
            if (key == "schema_version") {
                if (parsed.value() != 1)
                    return invalid(line_number, "Unsupported configuration schema.");
                config.schema_version = 1;
            } else if (key == "worker_limit")
                config.limits.worker_limit = parsed.value();
            else
                config.limits.queue_limit = parsed.value();
        } else if (key == "log_level") {
            if (value == "debug")
                config.log_level = LogLevel::debug;
            else if (value == "info")
                config.log_level = LogLevel::info;
            else if (value == "warning")
                config.log_level = LogLevel::warning;
            else if (value == "error")
                config.log_level = LogLevel::error;
            else if (value == "off")
                config.log_level = LogLevel::off;
            else
                return invalid(line_number, "Unknown log level.");
        } else if (key == "privacy_mode") {
            if (value == "direct")
                config.privacy_mode = PrivacyMode::direct;
            else if (value == "protected")
                config.privacy_mode = PrivacyMode::protected_mode;
            else if (value == "privacy-first")
                config.privacy_mode = PrivacyMode::privacy_first;
            else
                return invalid(line_number, "Unknown privacy mode.");
        } else if (key == "data_root") {
            auto path = platform::path_from_utf8(value);
            if (!path)
                return invalid(line_number, "Invalid path encoding.");
            config.data_root = std::move(path).value();
        } else
            return invalid(line_number, "Unknown key.");
    }
    auto valid = validate_config(config);
    if (!valid)
        return valid.error();
    return config;
}
Result<Config> load_config(const std::filesystem::path &path) {
    auto text = platform::read_text_file(path, max_config_bytes);
    if (!text)
        return text.error();
    return parse_config(text.value());
}
} // namespace ghostwire::core
