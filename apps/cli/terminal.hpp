#pragma once

namespace ghostwire::cli {
// GW-APP-LOCAL-CONSOLE, owner: coordinator. Borrow standard input/output only;
// do not change terminal modes. Redirection or unavailable handles return false.
[[nodiscard]] bool interactive_terminal() noexcept;
} // namespace ghostwire::cli
