#include "ghostwire/platform/filesystem.hpp"
#include <array>
#include <cstdlib>
#include <fstream>
#include <memory>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// clang-format off
#include <windows.h>
#include <shlobj.h>
// clang-format on
#endif

namespace ghostwire::platform {
namespace fs = std::filesystem;
using core::Error;
using core::ErrorCode;
using core::Result;
namespace {
#ifdef _WIN32
struct ShellStringDeleter {
    void operator()(wchar_t *text) const noexcept { CoTaskMemFree(text); }
};
Result<fs::path> known_folder(REFKNOWNFOLDERID id) {
    PWSTR raw = nullptr;
    const auto status = SHGetKnownFolderPath(id, KF_FLAG_DONT_VERIFY, nullptr, &raw);
    std::unique_ptr<wchar_t, ShellStringDeleter> owned(raw);
    if (FAILED(status) || !owned)
        return Error{ErrorCode::filesystem, "Cannot resolve Windows user directory."};
    return fs::path(owned.get());
}
#else
fs::path environment_path(const char *key) {
    const auto *value = std::getenv(key);
    if (!value || !*value)
        return {};
    fs::path result(value);
    return result.is_absolute() ? result : fs::path{};
}
#endif
} // namespace

Result<fs::path> path_from_utf8(std::string_view text) {
    for (std::size_t i = 0; i < text.size();) {
        const auto first = static_cast<unsigned char>(text[i++]);
        if (first < 32 || first == 127)
            return Error{ErrorCode::filesystem, "Path contains a control character."};
        if (first < 128)
            continue;
        unsigned remaining{}, codepoint{}, minimum{};
        if (first >= 0xc2 && first <= 0xdf) {
            remaining = 1;
            codepoint = first & 0x1fU;
            minimum = 0x80;
        } else if (first >= 0xe0 && first <= 0xef) {
            remaining = 2;
            codepoint = first & 0x0fU;
            minimum = 0x800;
        } else if (first >= 0xf0 && first <= 0xf4) {
            remaining = 3;
            codepoint = first & 0x07U;
            minimum = 0x10000;
        } else
            return Error{ErrorCode::filesystem, "Path is not valid UTF-8."};
        if (remaining > text.size() - i)
            return Error{ErrorCode::filesystem, "Path is not valid UTF-8."};
        for (unsigned j = 0; j < remaining; ++j) {
            const auto next = static_cast<unsigned char>(text[i++]);
            if ((next & 0xc0U) != 0x80U)
                return Error{ErrorCode::filesystem, "Path is not valid UTF-8."};
            codepoint = (codepoint << 6) | (next & 0x3fU);
        }
        if (codepoint < minimum || codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff))
            return Error{ErrorCode::filesystem, "Path is not valid UTF-8."};
    }
    try {
        return fs::path(std::u8string(text.begin(), text.end()));
    } catch (const fs::filesystem_error &) {
        return Error{ErrorCode::filesystem, "Cannot decode path."};
    }
}
std::string path_to_utf8(const fs::path &path) {
    const auto text = path.generic_u8string();
    return std::string(text.begin(), text.end());
}

Result<UserDirectories> user_directories() {
#ifdef _WIN32
    auto local = known_folder(FOLDERID_LocalAppData);
    if (!local)
        return local.error();
    // Deliberately no automatic roaming of sensitive state between devices.
    return UserDirectories{local.value() / "Data", local.value() / "Config", local.value() / "Cache"};
#else
    const auto home = environment_path("HOME");
#ifdef __APPLE__
    if (home.empty())
        return Error{ErrorCode::filesystem, "Set an absolute HOME or use --data-root."};
    return UserDirectories{home / "Library/Application Support", home / "Library/Application Support",
                           home / "Library/Caches"};
#else
    auto data = environment_path("XDG_DATA_HOME");
    auto config = environment_path("XDG_CONFIG_HOME");
    auto cache = environment_path("XDG_CACHE_HOME");
    if (home.empty() && (data.empty() || config.empty() || cache.empty()))
        return Error{ErrorCode::filesystem, "Set absolute user directories or use --data-root."};
    if (data.empty())
        data = home / ".local/share";
    if (config.empty())
        config = home / ".config";
    if (cache.empty())
        cache = home / ".cache";
    return UserDirectories{data, config, cache};
#endif
#endif
}

Result<std::string> read_text_file(const fs::path &path, std::size_t max_bytes) {
    std::error_code ec;
    if (!fs::is_regular_file(path, ec) || ec)
        return Error{ErrorCode::filesystem, "Configuration must be an accessible regular file."};
    std::ifstream input(path, std::ios::binary);
    if (!input)
        return Error{ErrorCode::filesystem, "Cannot open configuration file."};
    std::array<char, 4096> block{};
    std::string result;
    while (input.read(block.data(), static_cast<std::streamsize>(block.size())) || input.gcount() > 0) {
        const auto count = static_cast<std::size_t>(input.gcount());
        if (result.size() > max_bytes || count > max_bytes - result.size())
            return Error{ErrorCode::configuration, "Configuration exceeds the read budget."};
        result.append(block.data(), count);
    }
    if (input.bad() || (!input.eof() && input.fail()))
        return Error{ErrorCode::filesystem, "Cannot read configuration file."};
    return result;
}

Result<void> ensure_directory(const fs::path &path) {
    if (!path.is_absolute())
        return Error{ErrorCode::filesystem, "Directory path must be absolute."};
    std::error_code ec;
    const auto normalized = path.lexically_normal();
    fs::path current = normalized.root_path();
    for (const auto &component : normalized.relative_path()) {
        current /= component;
        auto status = fs::symlink_status(current, ec);
        if (ec && ec != std::errc::no_such_file_or_directory)
            return Error{ErrorCode::filesystem, "Cannot inspect application directory."};
        ec.clear();
        if (fs::is_symlink(status))
            return Error{ErrorCode::filesystem, "Application directories must not contain symbolic links."};
        if (fs::exists(status)) {
            if (!fs::is_directory(status))
                return Error{ErrorCode::filesystem, "Application path is not a directory."};
            continue;
        }
        // No secret data is written here; this is not the future secure vault API.
        if (!fs::create_directory(current, ec) && ec)
            return Error{ErrorCode::filesystem, "Cannot create application directory."};
        status = fs::symlink_status(current, ec);
        if (ec || !fs::is_directory(status) || fs::is_symlink(status))
            return Error{ErrorCode::filesystem, "Application directory changed during initialization."};
    }
    return {};
}
} // namespace ghostwire::platform
