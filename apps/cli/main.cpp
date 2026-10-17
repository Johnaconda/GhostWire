#include "ghostwire/core/bootstrap.hpp"
#include "ghostwire/platform/arguments.hpp"
#include "identity_session.hpp"
#include <algorithm>
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
        if (std::find(args.begin(), args.end(), "--identity-session") != args.end())
            return ghostwire::cli::run_identity_session(args, std::cin, std::cout, std::cerr);
        const int status = ghostwire::core::run_cli(ghostwire::core::Role::cli, args, std::cout, std::cerr);
        if (status == 0 && args.size() == 1 && args[0] == "--help") {
            std::cout << "  --identity-session [--data-root ABS_PATH]\n"
                      << "                          Open an explicit volatile identity console; type help.\n";
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
