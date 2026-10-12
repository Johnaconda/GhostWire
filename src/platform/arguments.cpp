#include "ghostwire/platform/arguments.hpp"
#include "ghostwire/core/resource_limits.hpp"
#include <string_view>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <memory>
// clang-format off
#include <windows.h>
#include <shellapi.h>
// clang-format on
#endif

namespace ghostwire::platform {
core::Result<std::vector<std::string>> arguments(int argc, char **argv) {
    using core::Error;
    using core::ErrorCode;
    std::vector<std::string> result;
#ifdef _WIN32
    (void)argc;
    (void)argv;
    int count{};
    auto wide = CommandLineToArgvW(GetCommandLineW(), &count);
    struct Deleter {
        void operator()(wchar_t **ptr) const noexcept { LocalFree(ptr); }
    };
    std::unique_ptr<wchar_t *, Deleter> owned(wide);
    if (!owned)
        return Error{ErrorCode::internal, "Cannot decode command line."};
    if (count > 33)
        return Error{ErrorCode::usage, "Too many arguments."};
    for (int i = 1; i < count; ++i) {
        const int size =
            WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide[i], -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0 || size > static_cast<int>(core::max_argument_bytes + 1))
            return Error{ErrorCode::usage, "Invalid argument encoding or length."};
        std::string value(static_cast<std::size_t>(size), '\0');
        if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide[i], -1, value.data(), size, nullptr,
                                 nullptr))
            return Error{ErrorCode::usage, "Cannot decode command line."};
        value.pop_back();
        result.push_back(std::move(value));
    }
#else
    if (argc > 33 || argc < 1 || argv == nullptr)
        return Error{ErrorCode::usage, "Invalid argument count."};
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == nullptr)
            return Error{ErrorCode::usage, "Invalid argument."};
        std::string_view value(argv[i]);
        if (value.size() > core::max_argument_bytes)
            return Error{ErrorCode::usage, "Argument exceeds length limit."};
        result.emplace_back(value);
    }
#endif
    return result;
}
} // namespace ghostwire::platform
