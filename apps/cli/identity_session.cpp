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
    output << "Commands (use numbers printed by the latest relevant list/show):\n"
           << "  menu                               Show the short numbered menu\n"
           << "  help\n"
           << "  back | cancel | menu | ..          Return to the main menu from any prompt\n"
           << "  create                             Guided profile creation; cancel returns to commands\n"
           << "  create ALIAS DISPLAY_NAME          Create independent keys; select the new profile\n"
           << "  profiles                           Explicitly list local labels and device counts\n"
           << "  select PROFILE_NUMBER              Select this session's explicit profile handle\n"
           << "  select                             List profiles and ask which one to select\n"
           << "  status                             Show selected labels and counts; no public keys\n"
           << "  rename                             Guided label change for selected profile\n"
           << "  rename ALIAS DISPLAY_NAME          Rename selected profile without changing keys\n"
           << "  membership                         Guided credential preparation (not admission)\n"
           << "  membership COMMUNITY_KEY_HEX ALIAS DISPLAY_NAME\n"
           << "                                     Explicitly pin full key and prepare credentials\n"
           << "  memberships                        List selected profile's prepared credentials\n"
           << "  manage-membership [NUMBER]         Open membership label/device/removal actions\n"
           << "  rename-membership NUMBER [ALIAS DISPLAY_NAME]\n"
           << "                                     Rename prepared credentials; keys unchanged\n"
           << "  delete profile | delete membership NUMBER\n"
           << "                                     Remove local keys only after typed confirmation\n"
           << "  show profile | show membership MEMBERSHIP_NUMBER\n"
           << "                                     Explicitly disclose public keys/IDs/signed record\n"
           << "  add profile | add membership MEMBERSHIP_NUMBER\n"
           << "  rotate profile DEVICE_NUMBER | rotate membership MEMBERSHIP_NUMBER DEVICE_NUMBER\n"
           << "  revoke profile DEVICE_NUMBER | revoke membership MEMBERSHIP_NUMBER DEVICE_NUMBER\n"
           << "  trust-profile PROFILE_NUMBER       Explicitly pin and accept its current public set\n"
           << "  recognize-profile PROFILE_NUMBER DEVICE_NUMBER\n"
           << "                                     Check existing local trust; never auto-refresh\n"
           << "  block-profile PROFILE_NUMBER | unblock-profile PROFILE_NUMBER\n"
           << "  trusts                             List this profile's console-managed trust pins\n"
           << "  forget-trust TRUST_NUMBER          Confirm forgetting a pin and revision history\n"
           << "  diagnostics                        Counts only; no identifiers\n"
           << "  quit | exit                        Close from any prompt; clear volatile state\n"
           << "Aliases: 1-20 ASCII digits. Names: UTF-8, up to 256 bytes; spaces allowed.\n"
           << "Guided fields reserve navigation words. Use complete commands for literal word names.\n"
           << "Community keys: exactly 64 lowercase hex characters from an independently verified source.\n"
           << "Prepared membership is not community admission; device recognition is not a login.\n";
}

void menu(std::ostream &output) {
    output << "Local identity menu:\n"
           << "  1  Create profile\n"
           << "  2  List profiles\n"
           << "  3  Select profile\n"
           << "  4  Selected profile status\n"
           << "  5  List prepared memberships\n"
           << "  6  All commands\n"
           << "  7  Prepare membership credentials\n"
           << "  8  Manage a prepared membership\n"
           << "  9  Rename selected profile\n"
           << " 10  Delete selected profile\n"
           << "  0  Quit\n"
           << "Use a menu number or a command. Type menu to show this again.\n";
}

// GW-APP-PROMPT-NAVIGATION, owner: coordinator. All mutations use existing
// IdentityService operations. Pending actions bind full local owner/member/trust
// handles, never aliases or remembered list positions. Only bounded public input
// is staged. Deletion/forgetting require exact confirmation; cancel/EOF have no
// pending effects. No private key, executor or persistent store belongs to UI.
class Session {
  public:
    Session(identity::IdentityService &service, std::ostream &output, std::ostream &errors)
        : service_(service), output_(output), errors_(errors) {}

    // false means explicit quit. Recoverable command errors keep the session.
    bool command(std::string_view line) {
        // Check standalone navigation before a field can consume it as data.
        // Numeric 0 remains valid field data and keeps its existing menu meaning.
        auto navigation_tail = line;
        const auto navigation = token(navigation_tail);
        const bool standalone = remainder(navigation_tail).empty();
        if (standalone && (navigation == "quit" || navigation == "exit")) {
            reset_pending();
            return false;
        }
        if (pending_ != Pending::command) {
            if (standalone && (navigation == "cancel" || navigation == "back" || navigation == "menu" ||
                               navigation == "..")) {
                reset_pending();
                output_ << "Pending input cancelled; selected profile unchanged.\n";
                menu(output_);
                return true;
            }
            if (standalone && navigation == "help") {
                prompt_help();
                return true;
            }
            return guided_input(line);
        }
        auto name = token(line);
        if (remainder(line).empty()) {
            if (name == "0")
                name = "quit";
            else if (name == "1")
                name = "create";
            else if (name == "2")
                name = "profiles";
            else if (name == "3")
                name = "select";
            else if (name == "4")
                name = "status";
            else if (name == "5")
                name = "memberships";
            else if (name == "6")
                name = "help";
            else if (name == "7")
                name = "membership";
            else if (name == "8")
                name = "manage-membership";
            else if (name == "9")
                name = "rename";
            else if (name == "10") {
                name = "delete";
                line = "profile";
            }
        }
        if (name.empty())
            return true;
        if (name == "quit" && remainder(line).empty())
            return false;
        if (name == "help" && remainder(line).empty()) {
            help(output_);
            return true;
        }
        if ((name == "menu" || name == "cancel" || name == "back" || name == "..") &&
            remainder(line).empty()) {
            menu(output_);
            return true;
        }
        if (name == "status" && remainder(line).empty()) {
            if (!selected())
                return true;
            auto profile = service_.profile(*selected_);
            if (!check(profile))
                return true;
            auto memberships = service_.memberships(*selected_);
            if (check(memberships))
                output_ << "Selected profile: alias=" << profile.value().labels.alias
                        << " name=" << profile.value().labels.display_name
                        << " devices=" << profile.value().devices.size()
                        << " revision=" << profile.value().revision
                        << " prepared_memberships=" << memberships.value().size()
                        << "\nStorage: volatile; keys and trust state are cleared at exit.\n";
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
            list_profiles();
            return true;
        }
        if (name == "select") {
            if (remainder(line).empty()) {
                if (list_profiles()) {
                    pending_ = Pending::select_profile;
                    output_ << "Enter a profile number, or cancel to return to commands.\n";
                }
                return true;
            }
            const auto position = index(token(line));
            if (!position || !remainder(line).empty())
                return syntax();
            select_profile(*position);
            return true;
        }
        if (name == "create" || name == "rename") {
            if (remainder(line).empty()) {
                if (name == "rename" && !selected())
                    return true;
                pending_owner_ = name == "rename" ? selected_ : std::nullopt;
                begin_labels(name == "create" ? LabelAction::create_profile : LabelAction::rename_profile);
                return true;
            }
            const auto alias = token(line);
            const auto display_name = remainder(line);
            if (alias.empty() || display_name.empty())
                return syntax();
            if (name == "create") {
                create_profile(alias, display_name);
            } else if (selected()) {
                auto renamed = service_.rename_profile(*selected_, alias, display_name);
                if (check(renamed))
                    output_ << "Profile labels changed; identity unchanged.\n";
            }
            return true;
        }
        if (name == "membership") {
            if (remainder(line).empty()) {
                if (selected()) {
                    pending_owner_ = *selected_;
                    pending_ = Pending::community_key;
                    output_ << "Prepare independent membership credentials; this does not join a server.\n"
                            << "Use the full community public key from an independently verified source.\n"
                            << "No pin or credentials are created until all fields are submitted.\n"
                            << "Enter cancel at any prompt to discard pending input.\n";
                }
                return true;
            }
            const auto community_key = public_key(token(line));
            const auto alias = token(line);
            const auto display_name = remainder(line);
            if (!community_key || alias.empty() || display_name.empty())
                return syntax();
            if (!selected())
                return true;
            prepare_membership(*selected_, *community_key, alias, display_name);
            return true;
        }
        if (name == "memberships" && remainder(line).empty()) {
            list_memberships();
            return true;
        }
        if (name == "manage-membership" || name == "rename-membership") {
            if (!selected())
                return true;
            if (name == "manage-membership" && remainder(line).empty()) {
                if (list_memberships()) {
                    pending_owner_ = *selected_;
                    pending_ = Pending::membership_selection;
                    output_ << "Enter a membership number, or cancel.\n";
                }
                return true;
            }
            const auto position = index(token(line));
            if (!position || (name == "manage-membership" && !remainder(line).empty()))
                return syntax();
            auto member = membership_at(*selected_, *position);
            if (!member)
                return true;
            if (name == "manage-membership") {
                pending_owner_ = *selected_;
                pending_member_ = member->handle;
                managing_member_ = true;
                member_panel();
            } else if (remainder(line).empty()) {
                pending_owner_ = *selected_;
                pending_member_ = member->handle;
                begin_labels(LabelAction::rename_membership);
            } else {
                const auto alias = token(line);
                const auto display_name = remainder(line);
                if (display_name.empty())
                    return syntax();
                if (check(service_.rename_membership(*selected_, member->handle, alias, display_name)))
                    output_ << "Membership labels changed; identity unchanged.\n";
            }
            return true;
        }
        if (name == "delete") {
            const auto scope = token(line);
            if (scope == "profile" && remainder(line).empty()) {
                if (selected()) {
                    pending_owner_ = *selected_;
                    begin_delete(false);
                }
            } else if (scope == "membership") {
                const auto position = index(token(line));
                if (!position || !remainder(line).empty())
                    return syntax();
                if (!selected())
                    return true;
                auto member = membership_at(*selected_, *position);
                if (member) {
                    pending_owner_ = *selected_;
                    pending_member_ = member->handle;
                    begin_delete(true);
                }
            } else
                return syntax();
            return true;
        }
        if (name == "trusts" && remainder(line).empty()) {
            list_trusts();
            return true;
        }
        if (name == "forget-trust") {
            const auto position = index(token(line));
            if (!position || !remainder(line).empty())
                return syntax();
            if (!selected())
                return true;
            if (*position >= trust_.size() || !trust_[*position] || trust_[*position]->owner != *selected_)
                return invalid_selection();
            pending_owner_ = *selected_;
            pending_trust_ = trust_[*position]->handle;
            pending_ = Pending::confirm_forget;
            output_ << "Forget this profile's selected trust pin and its revision history.\n"
                    << "Type forget to confirm, or cancel to keep it.\n";
            return true;
        }
        if (name == "show" || name == "add" || name == "rotate" || name == "revoke")
            return device_command(name, line);
        if (name == "trust-profile" || name == "recognize-profile" || name == "block-profile" ||
            name == "unblock-profile")
            return trust_command(name, line);
        return syntax();
    }

    void prompt() {
        switch (pending_) {
        case Pending::command:
            output_ << "ghostwire> ";
            break;
        case Pending::create_alias:
            output_ << "numeric alias (1-20 digits; back/cancel/quit)> ";
            break;
        case Pending::create_name:
            output_ << "display name (UTF-8, 1-256 bytes; back/cancel/quit)> ";
            break;
        case Pending::select_profile:
            output_ << "profile number> ";
            break;
        case Pending::community_key:
            output_ << "community public key> ";
            break;
        case Pending::membership_selection:
            output_ << "membership number> ";
            break;
        case Pending::membership_action:
            output_ << "membership action> ";
            break;
        case Pending::membership_device:
            output_ << "device number> ";
            break;
        case Pending::confirm_delete:
            output_ << "confirm (delete/cancel)> ";
            break;
        case Pending::confirm_forget:
            output_ << "confirm (forget/cancel)> ";
            break;
        }
        output_ << std::flush;
    }

  private:
    enum class Pending {
        command,
        create_alias,
        create_name,
        select_profile,
        community_key,
        membership_selection,
        membership_action,
        membership_device,
        confirm_delete,
        confirm_forget
    };
    enum class LabelAction { create_profile, rename_profile, prepare_membership, rename_membership };

    void reset_pending() {
        pending_ = Pending::command;
        pending_alias_.clear();
        pending_owner_.reset();
        pending_member_.reset();
        pending_key_.reset();
        pending_trust_.reset();
        managing_member_ = false;
    }

    void begin_labels(LabelAction action) {
        label_action_ = action;
        pending_alias_.clear();
        pending_ = Pending::create_alias;
        output_ << "Numeric alias: 1-20 digits, e.g. 123. Display name: visible text, e.g. Alex.\n"
                << "Use back/cancel for the main menu, help for this field, or quit to close.\n";
    }

    void prompt_help() {
        switch (pending_) {
        case Pending::create_alias:
            output_ << "Enter a numeric alias: 1-20 ASCII digits, e.g. 123. Your visible name comes next.\n";
            break;
        case Pending::create_name:
            output_ << "Enter your visible display name: 1-256 UTF-8 bytes, e.g. Alex. Spaces are allowed.\n";
            break;
        case Pending::select_profile:
            output_ << "Enter a profile number printed by the latest profile list.\n";
            break;
        case Pending::community_key:
            output_ << "Enter all 64 lowercase hex characters of an independently verified community key.\n";
            break;
        case Pending::membership_selection:
            output_ << "Enter a membership number printed by the latest membership list.\n";
            break;
        case Pending::membership_action:
            output_ << "Membership actions: 1 status, 2 rename, 3 add device, 4 rotate, 5 revoke,\n"
                    << "6 delete, 7 show public record/devices, 0 back.\n";
            break;
        case Pending::membership_device:
            output_ << "Enter a current device number from this membership's numbered public device list.\n";
            break;
        case Pending::confirm_delete:
            output_ << "Type exactly delete to confirm the displayed local key deletion.\n";
            break;
        case Pending::confirm_forget:
            output_ << "Type exactly forget to discard the selected trust pin and revision history.\n";
            break;
        case Pending::command:
            help(output_);
            break;
        }
        output_ << "Navigation: back/cancel/menu/.. return to the main menu; quit/exit close the console.\n"
                << "Navigation words are reserved here; complete commands can set literal word names.\n";
    }

    bool guided_input(std::string_view line) {
        if (pending_ == Pending::community_key) {
            const auto key = public_key(line);
            if (!key) {
                errors_ << "Identity command error: Community key requires 64 lowercase hex characters.\n";
                return true;
            }
            pending_key_ = *key;
            begin_labels(LabelAction::prepare_membership);
        } else if (pending_ == Pending::create_alias) {
            if (line.empty() || line.size() > 20 ||
                !std::all_of(line.begin(), line.end(), [](char c) { return c >= '0' && c <= '9'; })) {
                errors_ << "Identity command error: Alias requires 1-20 ASCII digits. "
                        << "Use e.g. 123; help shows examples, back/cancel returns, quit closes.\n";
                return true;
            }
            pending_alias_ = line;
            pending_ = Pending::create_name;
        } else if (pending_ == Pending::create_name) {
            if (remainder(line).empty() || line.size() > 256) {
                errors_ << "Identity command error: Display name requires 1-256 UTF-8 bytes.\n";
                return true;
            }
            switch (label_action_) {
            case LabelAction::create_profile:
                create_profile(pending_alias_, line);
                break;
            case LabelAction::rename_profile:
                if (check(service_.rename_profile(*pending_owner_, pending_alias_, line)))
                    output_ << "Profile labels changed; identity unchanged.\n";
                break;
            case LabelAction::prepare_membership:
                prepare_membership(*pending_owner_, *pending_key_, pending_alias_, line);
                break;
            case LabelAction::rename_membership:
                if (check(
                        service_.rename_membership(*pending_owner_, *pending_member_, pending_alias_, line)))
                    output_ << "Membership labels changed; identity unchanged.\n";
                break;
            }
            pending_alias_.clear();
            if (managing_member_)
                member_panel();
            else
                reset_pending();
        } else if (pending_ == Pending::select_profile || pending_ == Pending::membership_selection) {
            const auto position = index(token(line));
            if (!position || !remainder(line).empty())
                return invalid_selection();
            if (pending_ == Pending::select_profile) {
                if (select_profile(*position))
                    reset_pending();
            } else {
                auto member = membership_at(*pending_owner_, *position);
                if (member) {
                    pending_member_ = member->handle;
                    managing_member_ = true;
                    member_panel();
                }
            }
        } else if (pending_ == Pending::membership_action) {
            return member_action(line);
        } else if (pending_ == Pending::membership_device) {
            const auto position = index(token(line));
            if (!position || !remainder(line).empty())
                return invalid_selection();
            auto member = service_.membership(*pending_owner_, *pending_member_);
            if (!check(member)) {
                reset_pending();
                return true;
            }
            if (*position >= member.value().devices.size())
                return invalid_selection();
            const auto device = member.value().devices[*position].id;
            if (rotate_member_device_) {
                if (check(service_.rotate_membership_device(*pending_owner_, *pending_member_, device,
                                                            trusted_system_time())))
                    output_ << "Membership device replaced; old device revoked in this scope.\n";
            } else if (check(service_.revoke_membership_device(*pending_owner_, *pending_member_, device,
                                                               trusted_system_time())))
                output_ << "Membership device revoked; other scopes are unchanged.\n";
            member_panel();
        } else if (pending_ == Pending::confirm_delete) {
            if (line != "delete") {
                errors_ << "Identity command error: Type exactly delete, or cancel.\n";
                return true;
            }
            if (pending_member_) {
                if (check(service_.delete_membership(*pending_owner_, *pending_member_)))
                    output_ << "Local membership deleted; community pin retained; other scopes unchanged.\n";
            } else if (check(service_.delete_profile(*pending_owner_))) {
                for (auto &entry : trust_)
                    if (entry && entry->owner == *pending_owner_)
                        entry.reset();
                if (selected_ && *selected_ == *pending_owner_)
                    selected_.reset();
                output_ << "Local profile and its membership keys deleted; no profile selected.\n"
                        << "Other profiles and their independent trust pins are unchanged.\n";
            }
            reset_pending();
        } else if (pending_ == Pending::confirm_forget) {
            if (line != "forget") {
                errors_ << "Identity command error: Type exactly forget, or cancel.\n";
                return true;
            }
            if (check(service_.forget_authority(*pending_owner_, *pending_trust_))) {
                for (auto &entry : trust_)
                    if (entry && entry->owner == *pending_owner_ && entry->handle == *pending_trust_)
                        entry.reset();
                output_ << "Trust pin forgotten locally; no private keys changed.\n";
            }
            reset_pending();
        }
        return true;
    }

    void prepare_membership(identity::ProfileHandle owner, const crypto::PublicKey &key,
                            std::string_view alias, std::string_view display_name) {
        auto community = service_.pin_community(owner, key);
        if (!check(community))
            return;
        auto prepared =
            service_.prepare_membership(owner, community.value(), alias, display_name, trusted_system_time());
        if (check(prepared))
            output_ << "Community key pinned; independent membership credentials prepared (not admitted).\n";
        else
            output_ << "Community pin retained; membership preparation failed.\n";
    }

    bool list_memberships() {
        if (!selected())
            return false;
        auto values = service_.memberships(*selected_);
        if (!check(values))
            return false;
        output_ << "Prepared memberships: " << values.value().size() << '\n';
        for (std::size_t i = 0; i < values.value().size(); ++i) {
            const auto &value = values.value()[i];
            output_ << "Membership " << i + 1 << ": alias=" << value.labels.alias
                    << " name=" << value.labels.display_name << " devices=" << value.devices.size()
                    << " revision=" << value.revision << '\n';
        }
        if (values.value().empty())
            output_ << "No prepared memberships. Use 7 or membership to prepare credentials.\n";
        return !values.value().empty();
    }

    std::optional<identity::MembershipSnapshot> membership_at(identity::ProfileHandle owner,
                                                              std::size_t position) {
        auto values = service_.memberships(owner);
        if (!check(values))
            return std::nullopt;
        if (position >= values.value().size()) {
            invalid_selection();
            return std::nullopt;
        }
        return std::move(values.value()[position]);
    }

    void member_panel() {
        auto member = service_.membership(*pending_owner_, *pending_member_);
        if (!check(member)) {
            reset_pending();
            return;
        }
        pending_ = Pending::membership_action;
        output_ << "Managed membership: alias=" << member.value().labels.alias
                << " name=" << member.value().labels.display_name
                << " devices=" << member.value().devices.size() << " revision=" << member.value().revision
                << " (prepared, not admitted)\n"
                << "  1 Status              2 Rename labels\n"
                << "  3 Add device          4 Rotate device\n"
                << "  5 Revoke device       6 Delete membership\n"
                << "  7 Show public record/devices\n"
                << "  0 Back\n"
                << "Device numbers are 1-based positions in this membership's current device list.\n";
    }

    bool member_action(std::string_view line) {
        const auto action = token(line);
        if (!remainder(line).empty())
            return syntax();
        if (action == "0") {
            reset_pending();
            output_ << "Membership actions closed. Type menu for the main menu.\n";
        } else if (action == "1") {
            member_panel();
        } else if (action == "2") {
            begin_labels(LabelAction::rename_membership);
        } else if (action == "3") {
            if (check(
                    service_.add_membership_device(*pending_owner_, *pending_member_, trusted_system_time())))
                output_ << "Membership device added.\n";
            member_panel();
        } else if (action == "4" || action == "5") {
            rotate_member_device_ = action == "4";
            pending_ = Pending::membership_device;
            output_ << "Enter a current device number, or cancel to return to commands.\n";
        } else if (action == "6") {
            begin_delete(true);
        } else if (action == "7") {
            auto value = service_.membership(*pending_owner_, *pending_member_);
            if (check(value)) {
                auto record = service_.membership_record(*pending_owner_, *pending_member_);
                if (check(record)) {
                    output_ << "community=";
                    hex(output_, value.value().community.bytes);
                    output_ << '\n';
                    show(value.value(), record.value());
                }
            }
            member_panel();
        } else
            return syntax();
        return true;
    }

    void begin_delete(bool membership) {
        if (membership) {
            auto value = service_.membership(*pending_owner_, *pending_member_);
            if (!check(value)) {
                reset_pending();
                return;
            }
            output_ << "Delete local membership: alias=" << value.value().labels.alias
                    << " name=" << value.value().labels.display_name << '\n'
                    << "Its authority/device keys will be lost; this does not leave a remote server.\n";
        } else {
            pending_member_.reset();
            auto value = service_.profile(*pending_owner_);
            if (!check(value)) {
                reset_pending();
                return;
            }
            output_ << "Delete local profile: alias=" << value.value().labels.alias
                    << " name=" << value.value().labels.display_name << '\n'
                    << "All keys, memberships and trust owned by this profile will be lost.\n";
        }
        pending_ = Pending::confirm_delete;
        output_ << "Type delete to confirm, or cancel to keep it. No recovery is available.\n";
    }

    void list_trusts() {
        if (!selected())
            return;
        auto profiles = service_.profiles();
        if (!check(profiles))
            return;
        std::size_t count = 0;
        for (std::size_t i = 0; i < trust_.size(); ++i) {
            if (!trust_[i] || trust_[i]->owner != *selected_)
                continue;
            auto value = service_.trust(*selected_, trust_[i]->handle);
            if (!check(value))
                continue;
            ++count;
            output_ << "Trust " << i + 1 << ": subject_profile=";
            const auto subject =
                std::find_if(profiles.value().begin(), profiles.value().end(),
                             [&](const auto &entry) { return entry.handle == trust_[i]->subject; });
            if (subject == profiles.value().end())
                output_ << "deleted";
            else
                output_ << static_cast<std::size_t>(subject - profiles.value().begin()) + 1;
            constexpr std::array<std::string_view, 5> states{"unknown", "pinned", "independently_verified",
                                                             "blocked", "conflict"};
            output_ << " state=" << states[static_cast<std::size_t>(value.value().state)]
                    << " revision=" << value.value().highest_revision
                    << " has_record=" << (value.value().has_record ? "true" : "false") << '\n';
        }
        output_ << "Console-managed trust pins for selected profile: " << count << '\n';
    }

    void create_profile(std::string_view alias, std::string_view display_name) {
        auto created = service_.create_profile(alias, display_name, trusted_system_time());
        if (check(created)) {
            selected_ = std::move(created).value();
            output_ << "Profile created and selected.\n";
        }
    }

    bool list_profiles() {
        auto values = service_.profiles();
        if (!check(values))
            return false;
        output_ << "Local profiles: " << values.value().size() << '\n';
        for (std::size_t i = 0; i < values.value().size(); ++i) {
            const auto &value = values.value()[i];
            output_ << "Profile " << i + 1 << ": alias=" << value.labels.alias
                    << " name=" << value.labels.display_name << " devices=" << value.devices.size()
                    << " revision=" << value.revision;
            if (selected_ && value.handle == *selected_)
                output_ << " [selected]";
            output_ << '\n';
        }
        if (values.value().empty())
            output_ << "No profiles yet. Use 1 or create to create one.\n";
        return !values.value().empty();
    }

    bool select_profile(std::size_t position) {
        auto values = service_.profiles();
        if (!check(values))
            return false;
        if (position >= values.value().size()) {
            invalid_selection();
            return false;
        }
        selected_ = values.value()[position].handle;
        output_ << "Profile selected.\n";
        return true;
    }

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
    Pending pending_ = Pending::command;
    LabelAction label_action_ = LabelAction::create_profile;
    std::string pending_alias_;
    std::optional<identity::ProfileHandle> pending_owner_;
    std::optional<identity::MembershipHandle> pending_member_;
    std::optional<identity::TrustHandle> pending_trust_;
    std::optional<crypto::PublicKey> pending_key_;
    bool managing_member_ = false;
    bool rotate_member_device_ = false;
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
        << "GhostWire local identity console (volatile).\n"
        << "No identity is created until you request one. All keys and trust state are lost on exit.\n"
        << "No networking or private-key storage. Explicit console output may remain in terminal capture.\n"
        << "Type help for commands; quit or end-of-input closes the session.\n";
    output
        << "At guided prompts: back/cancel returns to the main menu; help explains the field; quit closes.\n";
    menu(output);
    bool io_failed = false;
    {
        Session session(*identity_adapter->identity(), output, errors);
        std::string line;
        line.reserve(maximum_line_bytes);
        for (;;) {
            session.prompt();
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
