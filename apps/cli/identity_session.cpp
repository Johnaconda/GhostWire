#include "identity_session.hpp"
#include "ghostwire/composition/identity_service_adapter.hpp"
#include "ghostwire/core/application.hpp"
#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <optional>
#include <string>

namespace ghostwire::cli {
namespace {
namespace core = ghostwire::core;
namespace identity = ghostwire::identity;
constexpr std::size_t maximum_line_bytes = 4096;

int report(const core::Error &error, std::ostream &errors) {
    errors << "GhostWire error " << core::exit_code(error.code) << ": " << error.message << '\n';
    return core::exit_code(error.code);
}

std::string_view token(std::string_view &input) {
    const auto start = input.find_first_not_of(" \t");
    if (start == std::string_view::npos) {
        input = {};
        return {};
    }
    input.remove_prefix(start);
    const auto end = input.find_first_of(" \t");
    const auto result = input.substr(0, end);
    input = end == std::string_view::npos ? std::string_view{} : input.substr(end + 1);
    return result;
}

std::string_view remainder(std::string_view input) {
    const auto first = input.find_first_not_of(" \t");
    return first == std::string_view::npos ? std::string_view{} : input.substr(first);
}

std::optional<std::size_t> index(std::string_view text) {
    if (text.empty())
        return std::nullopt;
    std::size_t value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value == 0)
        return std::nullopt;
    return value - 1;
}

std::optional<crypto::PublicKey> public_key(std::string_view text) {
    crypto::PublicKey key{};
    if (text.size() != key.size() * 2)
        return std::nullopt;
    constexpr std::string_view digits = "0123456789abcdef";
    for (std::size_t i = 0; i < key.size(); ++i) {
        const auto high = digits.find(text[2 * i]);
        const auto low = digits.find(text[2 * i + 1]);
        if (high == std::string_view::npos || low == std::string_view::npos)
            return std::nullopt;
        key[i] = static_cast<std::uint8_t>(high * 16 + low);
    }
    return key;
}

void hex(std::ostream &output, std::span<const std::uint8_t> bytes) {
    constexpr std::string_view digits = "0123456789abcdef";
    for (const auto byte : bytes)
        output << digits[byte >> 4U] << digits[byte & 15U];
}

identity::VerificationTime trusted_system_time() noexcept {
    const auto now =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    if (now < 0 || static_cast<std::uint64_t>(now) > identity::maximum_utc_seconds)
        return {};
    // The local composition owner chooses OS UTC as its trusted source. Service
    // rollback detection remains active; this does not assert authenticated time.
    return {static_cast<std::uint64_t>(now), true};
}

enum class LineResult { line, eof, too_long, failure };
LineResult read_line(std::istream &input, std::string &line) {
    line.clear();
    bool overflow = false;
    for (;;) {
        const auto next = input.get();
        if (next == std::char_traits<char>::eof()) {
            if (input.bad() || !input.eof())
                return LineResult::failure;
            if (overflow)
                return LineResult::too_long;
            return line.empty() ? LineResult::eof : LineResult::line;
        }
        if (next == '\n') {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            return overflow ? LineResult::too_long : LineResult::line;
        }
        if (line.size() < maximum_line_bytes)
            line.push_back(static_cast<char>(next));
        else
            overflow = true; // Drain this line without retaining more input.
    }
}

void help(std::ostream &output) {
    output << "Commands (numbers are 1-based positions from the latest list/show):\n"
           << "  help\n"
           << "  create ALIAS DISPLAY_NAME          Create independent keys; select the new profile\n"
           << "  profiles                           Explicitly list local labels and device counts\n"
           << "  select PROFILE_NUMBER              Select this session's explicit profile handle\n"
           << "  rename ALIAS DISPLAY_NAME          Rename selected profile without changing keys\n"
           << "  membership COMMUNITY_KEY_HEX ALIAS DISPLAY_NAME\n"
           << "                                     Explicitly pin full key and prepare credentials\n"
           << "  memberships                        List selected profile's prepared credentials\n"
           << "  show profile | show membership MEMBERSHIP_NUMBER\n"
           << "                                     Explicitly disclose public keys/IDs/signed record\n"
           << "  add profile | add membership MEMBERSHIP_NUMBER\n"
           << "  rotate profile DEVICE_NUMBER | rotate membership MEMBERSHIP_NUMBER DEVICE_NUMBER\n"
           << "  revoke profile DEVICE_NUMBER | revoke membership MEMBERSHIP_NUMBER DEVICE_NUMBER\n"
           << "  trust-profile PROFILE_NUMBER       Explicitly pin and accept its current public set\n"
           << "  recognize-profile PROFILE_NUMBER DEVICE_NUMBER\n"
           << "                                     Check existing local trust; never auto-refresh\n"
           << "  block-profile PROFILE_NUMBER | unblock-profile PROFILE_NUMBER\n"
           << "  diagnostics                        Counts only; no identifiers\n"
           << "  quit                               Clear owned volatile state and exit\n"
           << "Aliases: 1-20 ASCII digits. Names: UTF-8, up to 256 bytes; spaces allowed.\n"
           << "Community keys: exactly 64 lowercase hex characters from an independently verified source.\n"
           << "Prepared membership is not community admission; device recognition is not a login.\n";
}

class Session {
  public:
    Session(identity::IdentityService &service, std::ostream &output, std::ostream &errors)
        : service_(service), output_(output), errors_(errors) {}

    // false means explicit quit. Recoverable command errors keep the session.
    bool command(std::string_view line) {
        const auto name = token(line);
        if (name.empty())
            return true;
        if (name == "quit" && remainder(line).empty())
            return false;
        if (name == "help" && remainder(line).empty()) {
            help(output_);
            return true;
        }
        if (name == "diagnostics" && remainder(line).empty()) {
            const auto value = service_.diagnostic();
            output_ << "profiles=" << value.profiles << " memberships=" << value.memberships
                    << " devices=" << value.devices << " private_keys=" << value.private_keys
                    << " trust_contexts=" << value.trust_contexts
                    << " public_record_bytes=" << value.public_record_bytes
                    << " persistence_available=false\n";
            return true;
        }
        if (name == "profiles" && remainder(line).empty()) {
            auto values = service_.profiles();
            if (check(values)) {
                output_ << "Local profiles: " << values.value().size() << '\n';
                for (std::size_t i = 0; i < values.value().size(); ++i) {
                    const auto &value = values.value()[i];
                    output_ << "Profile " << i + 1 << ": alias=" << value.labels.alias
                            << " name=" << value.labels.display_name << " devices=" << value.devices.size()
                            << " revision=" << value.revision << '\n';
                }
            }
            return true;
        }
        if (name == "select") {
            const auto position = index(token(line));
            if (!position || !remainder(line).empty())
                return syntax();
            auto values = service_.profiles();
            if (check(values)) {
                if (*position >= values.value().size())
                    return invalid_selection();
                selected_ = values.value()[*position].handle;
                output_ << "Profile selected.\n";
            }
            return true;
        }
        if (name == "create" || name == "rename") {
            const auto alias = token(line);
            const auto display_name = remainder(line);
            if (alias.empty() || display_name.empty())
                return syntax();
            if (name == "create") {
                auto created = service_.create_profile(alias, display_name, trusted_system_time());
                if (check(created)) {
                    selected_ = std::move(created).value();
                    output_ << "Profile created and selected.\n";
                }
            } else if (selected()) {
                auto renamed = service_.rename_profile(*selected_, alias, display_name);
                if (check(renamed))
                    output_ << "Profile labels changed; identity unchanged.\n";
            }
            return true;
        }
        if (name == "membership") {
            const auto community_key = public_key(token(line));
            const auto alias = token(line);
            const auto display_name = remainder(line);
            if (!community_key || alias.empty() || display_name.empty())
                return syntax();
            if (!selected())
                return true;
            auto community = service_.pin_community(*selected_, *community_key);
            if (!check(community))
                return true;
            auto prepared = service_.prepare_membership(*selected_, community.value(), alias, display_name,
                                                        trusted_system_time());
            if (check(prepared))
                output_
                    << "Community key pinned; independent membership credentials prepared (not admitted).\n";
            else
                output_ << "Community pin retained; membership preparation failed.\n";
            return true;
        }
        if (name == "memberships" && remainder(line).empty()) {
            if (!selected())
                return true;
            auto values = service_.memberships(*selected_);
            if (check(values)) {
                output_ << "Prepared memberships: " << values.value().size() << '\n';
                for (std::size_t i = 0; i < values.value().size(); ++i) {
                    const auto &value = values.value()[i];
                    output_ << "Membership " << i + 1 << ": alias=" << value.labels.alias
                            << " name=" << value.labels.display_name << " devices=" << value.devices.size()
                            << " revision=" << value.revision << '\n';
                }
            }
            return true;
        }
        if (name == "show" || name == "add" || name == "rotate" || name == "revoke")
            return device_command(name, line);
        if (name == "trust-profile" || name == "recognize-profile" || name == "block-profile" ||
            name == "unblock-profile")
            return trust_command(name, line);
        return syntax();
    }

  private:
    template <class T> bool check(const identity::Result<T> &value) {
        if (!value)
            errors_ << "Identity command error: " << identity::describe(value.error().code) << '\n';
        return bool(value);
    }

    bool syntax() {
        errors_ << "Identity command error: Invalid command or arguments. Type help.\n";
        return true;
    }

    bool invalid_selection() {
        errors_ << "Identity command error: Selection is unavailable. List current entries first.\n";
        return true;
    }

    bool selected() {
        if (!selected_)
            errors_ << "Identity command error: Explicitly create or select a profile first.\n";
        return selected_.has_value();
    }

    template <class Snapshot> void show(const Snapshot &value, const identity::DeviceSetFrame &record) {
        output_ << "identity=";
        hex(output_, value.id.bytes);
        output_ << "\nauthority_public_key=";
        hex(output_, value.public_key);
        output_ << "\nrevision=" << value.revision << " devices=" << value.devices.size() << '\n';
        for (std::size_t i = 0; i < value.devices.size(); ++i) {
            output_ << "Device " << i + 1 << " id=";
            hex(output_, value.devices[i].id.bytes);
            output_ << " public_key=";
            hex(output_, value.devices[i].public_key);
            output_ << '\n';
        }
        output_ << "signed_public_record=";
        hex(output_, record.view());
        output_ << '\n';
    }

    bool device_command(std::string_view command, std::string_view args) {
        const auto scope = token(args);
        if (scope != "profile" && scope != "membership")
            return syntax();
        std::optional<std::size_t> membership_position;
        if (scope == "membership") {
            membership_position = index(token(args));
            if (!membership_position)
                return syntax();
        }
        std::optional<std::size_t> device_position;
        if (command == "rotate" || command == "revoke") {
            device_position = index(token(args));
            if (!device_position)
                return syntax();
        }
        if (!remainder(args).empty())
            return syntax();
        if (!selected())
            return true;
        if (scope == "profile") {
            auto value = service_.profile(*selected_);
            if (!check(value))
                return true;
            if (device_position && *device_position >= value.value().devices.size())
                return invalid_selection();
            if (command == "show") {
                auto record = service_.profile_record(*selected_);
                if (check(record))
                    show(value.value(), record.value());
            } else if (command == "add") {
                if (check(service_.add_profile_device(*selected_, trusted_system_time())))
                    output_ << "Profile device added.\n";
            } else if (command == "rotate") {
                if (check(service_.rotate_profile_device(
                        *selected_, value.value().devices[*device_position].id, trusted_system_time())))
                    output_ << "Profile device replaced; old device revoked in this scope.\n";
            } else if (check(service_.revoke_profile_device(
                           *selected_, value.value().devices[*device_position].id, trusted_system_time()))) {
                output_ << "Profile device revoked; independent membership devices are unchanged.\n";
            }
            return true;
        }
        auto values = service_.memberships(*selected_);
        if (!check(values))
            return true;
        if (*membership_position >= values.value().size())
            return invalid_selection();
        const auto &value = values.value()[*membership_position];
        if (device_position && *device_position >= value.devices.size())
            return invalid_selection();
        if (command == "show") {
            auto record = service_.membership_record(*selected_, value.handle);
            if (check(record)) {
                output_ << "community=";
                hex(output_, value.community.bytes);
                output_ << '\n';
                show(value, record.value());
            }
        } else if (command == "add") {
            if (check(service_.add_membership_device(*selected_, value.handle, trusted_system_time())))
                output_ << "Membership device added.\n";
        } else if (command == "rotate") {
            if (check(service_.rotate_membership_device(
                    *selected_, value.handle, value.devices[*device_position].id, trusted_system_time())))
                output_ << "Membership device replaced; old device revoked in this scope.\n";
        } else if (check(service_.revoke_membership_device(*selected_, value.handle,
                                                           value.devices[*device_position].id,
                                                           trusted_system_time()))) {
            output_ << "Membership device revoked; other scopes are unchanged.\n";
        }
        return true;
    }

    bool trust_command(std::string_view command, std::string_view args) {
        const auto profile_position = index(token(args));
        std::optional<std::size_t> device_position;
        if (command == "recognize-profile")
            device_position = index(token(args));
        if (!profile_position || (command == "recognize-profile" && !device_position) ||
            !remainder(args).empty())
            return syntax();
        if (!selected())
            return true;
        auto profiles = service_.profiles();
        if (!check(profiles))
            return true;
        if (*profile_position >= profiles.value().size())
            return invalid_selection();
        const auto &subject = profiles.value()[*profile_position];
        if (device_position && *device_position >= subject.devices.size())
            return invalid_selection();
        auto found = std::find_if(trust_.begin(), trust_.end(), [&](const auto &entry) {
            return entry && entry->owner == *selected_ && entry->subject == subject.handle;
        });
        if (command == "trust-profile") {
            if (found == trust_.end()) {
                found = std::find_if(trust_.begin(), trust_.end(), [](const auto &entry) { return !entry; });
                if (found == trust_.end()) {
                    errors_ << "Identity command error: Local trust selection budget is exhausted.\n";
                    return true;
                }
            }
            auto record = service_.profile_record(subject.handle);
            if (!check(record))
                return true;
            const identity::ExpectedAuthority expected{
                identity::AuthorityKind::profile, {}, subject.public_key};
            auto pinned = service_.pin_authority(*selected_, expected);
            if (!check(pinned))
                return true;
            *found = TrustSelection{*selected_, subject.handle, pinned.value()};
            if (check(service_.accept_record(*selected_, pinned.value(), record.value().view(),
                                             trusted_system_time())))
                output_ << "Exact profile authority pinned; current public device set accepted locally.\n";
            else
                output_ << "Explicit trust pin retained; public device set was not accepted.\n";
            return true;
        }
        if (found == trust_.end()) {
            errors_ << "Identity command error: No explicit trust selection exists for this profile.\n";
            return true;
        }
        if (command == "recognize-profile") {
            if (check(service_.recognize_device(*selected_, (*found)->handle,
                                                subject.devices[*device_position].public_key,
                                                trusted_system_time())))
                output_
                    << "Device recognized by current local trust; no login or action permission granted.\n";
        } else if (check(service_.set_blocked(*selected_, (*found)->handle, command == "block-profile"))) {
            output_ << (command == "block-profile" ? "Profile authority blocked locally.\n"
                                                   : "Profile authority unblocked locally.\n");
        }
        return true;
    }

    struct TrustSelection {
        identity::ProfileHandle owner;
        identity::ProfileHandle subject;
        identity::TrustHandle handle;
    };

    identity::IdentityService &service_;
    std::ostream &output_;
    std::ostream &errors_;
    std::optional<identity::ProfileHandle> selected_;
    std::array<std::optional<TrustSelection>, 256> trust_{};
};
} // namespace

int run_identity_session(std::span<const std::string_view> args, std::istream &input, std::ostream &output,
                         std::ostream &errors) {
    if ((args.size() != 1 && args.size() != 3) || args[0] != "--identity-session" ||
        (args.size() == 3 && args[1] != "--data-root"))
        return report({core::ErrorCode::usage, "Use --identity-session [--data-root ABS_PATH]."}, errors);
    core::Config config;
    if (args.size() == 3) {
        if (args[2].empty() || args[2].size() > core::max_argument_bytes || args[2].starts_with("--") ||
            std::any_of(args[2].begin(), args[2].end(), [](char value) {
                const auto byte = static_cast<unsigned char>(value);
                return byte < 32 || byte == 127;
            }))
            return report({core::ErrorCode::usage, "Invalid identity session path argument."}, errors);
        auto path = platform::path_from_utf8(args[2]);
        if (!path)
            return report({core::ErrorCode::usage, "Invalid path encoding."}, errors);
        config.data_root = std::move(path).value();
    }
    auto valid = core::validate_config(config);
    if (!valid)
        return report(valid.error(), errors);
    platform::UserDirectories defaults;
    if (config.data_root.empty()) {
        auto directories = platform::user_directories();
        if (!directories)
            return report(directories.error(), errors);
        defaults = std::move(directories).value();
    }
    auto paths = core::resolve_paths(core::Role::cli, config.data_root, defaults);
    if (!paths)
        return report(paths.error(), errors);
    core::Logger logger(errors, config.log_level);
    core::Application app(core::Role::cli, std::move(config), std::move(paths).value(), logger);
    auto crypto = std::make_unique<core::CryptoService>();
    auto adapter = std::make_unique<composition::IdentityServiceAdapter>(*crypto);
    auto *identity_adapter = adapter.get();
    auto registered = app.add_service(core::cryptographic_provider_service, std::move(crypto));
    if (!registered)
        return report(registered.error(), errors);
    constexpr std::array dependencies{core::cryptographic_provider_service};
    registered = app.add_service(composition::identity_service, std::move(adapter), dependencies);
    if (!registered)
        return report(registered.error(), errors);
    auto started = app.start();
    if (!started)
        return report(started.error(), errors);
    output
        << "GhostWire volatile identity session (A04-P03).\n"
        << "No identity is created until you request one. All keys and trust state are lost on exit.\n"
        << "No networking or private-key storage. Explicit show/list output may remain in terminal capture.\n"
        << "Type help for commands; quit or end-of-input closes the session.\n";
    bool io_failed = false;
    {
        Session session(*identity_adapter->identity(), output, errors);
        std::string line;
        line.reserve(maximum_line_bytes);
        for (;;) {
            output << "ghostwire> " << std::flush;
            if (!output.good() || !errors.good()) {
                io_failed = true;
                break;
            }
            const auto status = read_line(input, line);
            if (status == LineResult::eof)
                break;
            if (status == LineResult::failure) {
                io_failed = true;
                break;
            }
            if (status == LineResult::too_long) {
                errors << "Identity command error: Input line exceeds the 4096-byte limit.\n";
                continue;
            }
            if (!session.command(line))
                break;
        }
    } // All borrowed handles/consumer state end before adapter/provider shutdown.
    auto stopped = app.shutdown();
    if (!stopped)
        return report(stopped.error(), errors);
    output << "Identity session closed; volatile state cleared.\n";
    return io_failed || !output.good() || !errors.good() ? 70 : 0;
}
} // namespace ghostwire::cli
