#pragma once

#include "ghostwire/core/result.hpp"
#include <cstdint>
#include <string_view>

namespace ghostwire::core {
// Numeric project version only; protocol versions are a separate future contract.
struct Version {
    std::uint32_t major{}, minor{}, patch{};
    static Result<Version> parse(std::string_view text);
    [[nodiscard]] std::string to_string() const;
    bool operator==(const Version &) const = default;
};
std::string_view version_string() noexcept;
std::string_view checkpoint() noexcept;
} // namespace ghostwire::core
