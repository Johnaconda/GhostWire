#pragma once
#include <istream>
#include <ostream>
#include <span>
#include <string_view>

namespace ghostwire::cli {
// GW-APP-PROMPT-NAVIGATION, owner: coordinator. Owner-thread console, explicitly
// requested or opened by argument-free terminal launch. Input/output use UTF-8. No input
// history, key file, profile file, or persistent trust state is written.
int run_identity_session(std::span<const std::string_view> args, std::istream &input, std::ostream &output,
                         std::ostream &errors);
} // namespace ghostwire::cli
