#include "ghostwire/core/version.hpp"
#include "ghostwire/build_version.hpp"
#include <charconv>

namespace ghostwire::core {
Result<Version> Version::parse(std::string_view text) {
    Version result;
    std::uint32_t *fields[]{&result.major, &result.minor, &result.patch};
    for (unsigned i = 0; i < 3; ++i) {
        const auto dot = text.find('.');
        if ((i < 2 && dot == text.npos) || (i == 2 && dot != text.npos)) {
            return Error{ErrorCode::configuration, "Version must contain three numeric components."};
        }
        const auto field = text.substr(0, dot);
        if (field.empty() || (field.size() > 1 && field.front() == '0')) {
            return Error{ErrorCode::configuration, "Invalid numeric version component."};
        }
        const auto [end, ec] = std::from_chars(field.data(), field.data() + field.size(), *fields[i]);
        if (ec != std::errc{} || end != field.data() + field.size()) {
            return Error{ErrorCode::configuration, "Invalid numeric version component."};
        }
        if (i < 2)
            text.remove_prefix(dot + 1);
    }
    return result;
}
std::string Version::to_string() const {
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}
std::string_view version_string() noexcept {
    return GW_VERSION;
}
std::string_view checkpoint() noexcept {
    return GW_CHECKPOINT;
}
} // namespace ghostwire::core
