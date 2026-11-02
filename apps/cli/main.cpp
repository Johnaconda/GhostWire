#include "ghostwire/core/bootstrap.hpp"
#include "ghostwire/platform/arguments.hpp"
#include "identity_session.hpp"
#include "terminal.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

int main(int argc, char **argv) {
    try {
        auto arguments = ghostwire::platform::arguments(argc, argv);
        if (!arguments) {
            std::cerr << "GhostWire error " << ghostwire::core::exit_code(arguments.error().code) << ": "
                      << arguments.error().message << '\n';
            return ghostwire::core::exit_code(arguments.error().code);
        }
        std::vector<std::string_view> args;
        args.reserve(arguments.value().size());
        for (const auto &argument : arguments.value())
            args.emplace_back(argument);
        // GW-APP-LOCAL-CONSOLE, owner: coordinator. Only an argument-free,
        // attached terminal opts into the local console automatically. Explicit
        // commands and redirected bootstrap invocations keep their own route.
        if (args.empty() && ghostwire::cli::interactive_terminal()) {
            constexpr std::array<std::string_view, 1> session_args{"--identity-session"};
            return ghostwire::cli::run_identity_session(session_args, std::cin, std::cout, std::cerr);
        }
        if (std::find(args.begin(), args.end(), "--identity-session") != args.end())
            return ghostwire::cli::run_identity_session(args, std::cin, std::cout, std::cerr);
        const int status = ghostwire::core::run_cli(ghostwire::core::Role::cli, args, std::cout, std::cerr);
        if (status == 0 && args.size() == 1 && args[0] == "--help") {
            std::cout << "  --identity-session [--data-root ABS_PATH]\n"
                      << "                          Open the local console, including redirected commands.\n";
            return std::cout.good() ? 0 : 70;
        }
        return status;
    } catch (...) {
        // Exception text can contain paths or caller input.
        try {
            std::cerr << "GhostWire error 70: Unhandled bootstrap failure.\n";
        } catch (...) {
        }
        return 70;
    }
}
