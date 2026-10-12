#pragma once
#include "ghostwire/core/result.hpp"
#include <string>
#include <vector>

namespace ghostwire::platform {
// On Windows, obtains UTF-8 from the OS wide command line, not the ANSI argv.
core::Result<std::vector<std::string>> arguments(int argc, char **argv);
} // namespace ghostwire::platform
