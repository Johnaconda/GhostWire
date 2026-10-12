#include "ghostwire/core/bootstrap.hpp"
#include "ghostwire/core/application.hpp"
#include "ghostwire/core/version.hpp"
#include "ghostwire/platform/arguments.hpp"
#include "ghostwire/platform/platform_info.hpp"
#include <iostream>
#include <optional>
#include <set>
#include <vector>

namespace ghostwire::core {
namespace {
enum class Command { bootstrap, help, version, check_config, print_paths };
struct Options {
    Command command = Command::bootstrap;
    std::optional<std::filesystem::path> config;
    std::optional<std::filesystem::path> root;
};
Result<Options> parse_options(std::span<const std::string_view> args) {
    Options options;
    bool command_set = false;
    std::set<std::string_view> seen;
    if (args.size() > 32)
        return Error{ErrorCode::usage, "Too many arguments."};
    for (std::size_t i = 0; i < args.size(); ++i) {
        const auto arg = args[i];
        if (arg.size() > max_argument_bytes)
            return Error{ErrorCode::usage, "Argument exceeds length limit."};
        if (!seen.insert(arg).second)
            return Error{ErrorCode::usage, "Duplicate option."};
        if (arg == "--config" || arg == "--data-root") {
            if (++i >= args.size() || args[i].empty() || args[i].starts_with("--") ||
                args[i].size() > max_argument_bytes)
                return Error{ErrorCode::usage, "Path option requires a nonempty path argument."};
            for (const char code_unit : args[i]) {
                const auto byte = static_cast<unsigned char>(code_unit);
                if (byte < 32 || byte == 127)
                    return Error{ErrorCode::usage, "Path argument contains a control character."};
            }
            auto path = platform::path_from_utf8(args[i]);
            if (!path)
                return Error{ErrorCode::usage, "Invalid path encoding."};
            if (arg == "--config")
                options.config = std::move(path).value();
            else
                options.root = std::move(path).value();
            continue;
        }
        Command command;
        if (arg == "--help")
            command = Command::help;
        else if (arg == "--version")
            command = Command::version;
        else if (arg == "--check-config")
            command = Command::check_config;
        else if (arg == "--print-paths")
            command = Command::print_paths;
        else
            return Error{ErrorCode::usage, "Unknown option. Use --help."};
        if (command_set)
            return Error{ErrorCode::usage, "Choose only one command."};
        command_set = true;
        options.command = command;
    }
    if ((options.command == Command::help || options.command == Command::version) &&
        (options.config || options.root))
        return Error{ErrorCode::usage, "Help and version cannot be combined with configuration options."};
    return options;
}
int report(const Error &error, std::ostream &err) {
    err << "GhostWire error " << exit_code(error.code) << ": " << error.message << '\n';
    return exit_code(error.code);
}
void help(Role role, std::ostream &out) {
    out << "ghostwire-" << role_name(role) << " — A01 core lifecycle\n"
        << "Usage: ghostwire-" << role_name(role) << " [command] [options]\n"
        << "  --help                  Show this help\n"
        << "  --version               Show project/checkpoint/platform identity\n"
        << "  --check-config          Validate defaults or an explicit config; no writes\n"
        << "  --print-paths           Explicitly disclose resolved local paths; no writes\n"
        << "  --config PATH           Read this config (no implicit config search)\n"
        << "  --data-root ABS_PATH    Override storage roots; each role is isolated\n"
        << "Without a command: initialize directories, then shut down gracefully.\n"
        << "No GUI, listening socket, relay service, identity, vault or messaging exists in A01.\n"
        << "Exit codes: 0 success; 2 usage; 3 configuration; 4 filesystem; 70 internal.\n";
}
} // namespace

int run_cli(Role role, std::span<const std::string_view> args, std::ostream &out, std::ostream &err) {
    auto parsed = parse_options(args);
    if (!parsed)
        return report(parsed.error(), err);
    const auto &options = parsed.value();
    if (options.command == Command::help) {
        help(role, out);
        return out.good() ? 0 : 70;
    }
    if (options.command == Command::version) {
        const auto platform = platform::platform_info();
        out << "GhostWire " << version_string() << " | " << checkpoint() << " | " << role_name(role) << " | "
            << platform.operating_system << " | " << platform.architecture << '\n';
        return out.good() ? 0 : 70;
    }
    Config config;
    if (options.config) {
        auto loaded = load_config(*options.config);
        if (!loaded)
            return report(loaded.error(), err);
        config = std::move(loaded).value();
    }
    if (options.root)
        config.data_root = *options.root;
    auto valid = validate_config(config);
    if (!valid)
        return report(valid.error(), err);
    if (options.command == Command::check_config) {
        out << "Configuration valid (schema 1). Policy values are validated; A01 has no transport.\n";
        return out.good() ? 0 : 70;
    }
    platform::UserDirectories defaults;
    if (config.data_root.empty()) {
        auto directories = platform::user_directories();
        if (!directories)
            return report(directories.error(), err);
        defaults = std::move(directories).value();
    }
    auto paths = resolve_paths(role, config.data_root, defaults);
    if (!paths)
        return report(paths.error(), err);
    if (options.command == Command::print_paths) {
        out << "data=" << platform::path_to_utf8(paths.value().data)
            << "\nconfig=" << platform::path_to_utf8(paths.value().config)
            << "\ncache=" << platform::path_to_utf8(paths.value().cache)
            << "\nlogs=" << platform::path_to_utf8(paths.value().logs)
            << "\nruntime=" << platform::path_to_utf8(paths.value().runtime) << '\n';
        return out.good() ? 0 : 70;
    }
    Logger logger(err, config.log_level);
    Application app(role, std::move(config), std::move(paths).value(), logger);
    auto started = app.start();
    if (!started)
        return report(started.error(), err);
    out << "ghostwire-" << role_name(role)
        << ": A01 bootstrap complete; no live networking or encryption implemented.\n";
    auto stopped = app.shutdown();
    if (!stopped)
        return report(stopped.error(), err);
    return out.good() ? 0 : 70;
}
int bootstrap_main(Role role, int argc, char **argv) noexcept {
    try {
        auto owned = platform::arguments(argc, argv);
        if (!owned)
            return report(owned.error(), std::cerr);
        std::vector<std::string_view> args;
        args.reserve(owned.value().size());
        for (const auto &arg : owned.value())
            args.emplace_back(arg);
        return run_cli(role, args, std::cout, std::cerr);
    } catch (...) {
        // Never echo exception.what(): filesystem and other exceptions may include sensitive input.
        try {
            std::cerr << "GhostWire error 70: Unhandled bootstrap failure.\n";
        } catch (...) {
        }
        return 70;
    }
}
} // namespace ghostwire::core
