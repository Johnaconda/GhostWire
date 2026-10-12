#include "ghostwire/core/logger.hpp"
#include <array>
#include <sstream>

namespace ghostwire::core {
namespace {
std::string_view name(LogLevel level) {
    switch (level) {
    case LogLevel::debug:
        return "debug";
    case LogLevel::info:
        return "info";
    case LogLevel::warning:
        return "warning";
    case LogLevel::error:
        return "error";
    case LogLevel::off:
        return "off";
    }
    return "unknown";
}
std::string_view name(LogEvent event) {
    switch (event) {
    case LogEvent::startup:
        return "startup";
    case LogEvent::shutdown:
        return "shutdown";
    case LogEvent::configuration_validated:
        return "configuration_validated";
    case LogEvent::bootstrap_failed:
        return "bootstrap_failed";
    }
    return "unknown";
}
std::string_view name(LogKey key) {
    switch (key) {
    case LogKey::role:
        return "role";
    case LogKey::state:
        return "state";
    case LogKey::checkpoint:
        return "checkpoint";
    case LogKey::error_code:
        return "error_code";
    case LogKey::path:
        return "path";
    case LogKey::address:
        return "address";
    case LogKey::identity:
        return "identity";
    case LogKey::secret:
        return "secret";
    case LogKey::content:
        return "content";
    case LogKey::detail:
        return "detail";
    }
    return "unknown";
}
bool safe_key(LogKey key) {
    return key == LogKey::role || key == LogKey::state || key == LogKey::checkpoint ||
           key == LogKey::error_code;
}
void quoted(std::ostream &out, std::string_view text) {
    constexpr char digits[] = "0123456789abcdef";
    out << '"';
    const auto clipped = text.substr(0, 256);
    for (const char code_unit : clipped) {
        const auto c = static_cast<unsigned char>(code_unit);
        if (c == '"' || c == '\\')
            out << '\\' << static_cast<char>(c);
        else if (c < 32 || c >= 127)
            out << "\\u00" << digits[c >> 4] << digits[c & 15];
        else
            out << static_cast<char>(c);
    }
    if (text.size() > clipped.size())
        out << "[truncated]";
    out << '"';
}
} // namespace

Logger::Logger(std::ostream &sink, LogLevel threshold) : sink_(sink), threshold_(threshold) {}
bool Logger::write(LogLevel level, LogEvent event, std::initializer_list<LogField> fields) {
    if (level == LogLevel::off || level < threshold_)
        return true;
    std::ostringstream line;
    line << "{\"level\":";
    quoted(line, name(level));
    line << ",\"event\":";
    quoted(line, name(event));
    line << ",\"fields\":{";
    std::array<bool, 10> seen{};
    bool first = true;
    std::size_t count = 0;
    for (const auto &field : fields) {
        if (++count > 16)
            break;
        const auto index = static_cast<std::size_t>(field.key);
        if (index >= seen.size() || seen[index])
            continue;
        seen[index] = true;
        if (!first)
            line << ',';
        first = false;
        quoted(line, name(field.key));
        line << ':';
        quoted(line, field.classification == DataClass::public_metadata && safe_key(field.key)
                         ? field.value
                         : "[REDACTED]");
    }
    line << "}}\n";
    std::lock_guard lock(mutex_);
    try {
        sink_ << line.str();
        return sink_.good();
    } catch (const std::ios_base::failure &) {
        return false;
    }
}
bool Logger::flush() {
    std::lock_guard lock(mutex_);
    try {
        sink_.flush();
        return sink_.good();
    } catch (const std::ios_base::failure &) {
        return false;
    }
}
} // namespace ghostwire::core
