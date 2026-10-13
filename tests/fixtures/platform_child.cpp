#include "ghostwire/platform/arguments.hpp"
#include "ghostwire/platform/filesystem.hpp"
#include "ghostwire/platform/process_stop.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace {
std::string test_environment() {
#ifdef _WIN32
    wchar_t buffer[4096];
    const auto count = GetEnvironmentVariableW(L"GHOSTWIRE_CHILD_VALUE", buffer, 4096);
    if (!count)
        return "absent";
    if (count >= 4096)
        return "oversized";
    const auto bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buffer, static_cast<int>(count),
                                           nullptr, 0, nullptr, nullptr);
    std::string result(static_cast<std::size_t>(bytes), '\0');
    if (bytes && !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buffer, static_cast<int>(count),
                                      result.data(), bytes, nullptr, nullptr))
        return "invalid";
    return result;
#else
    const auto *value = std::getenv("GHOSTWIRE_CHILD_VALUE");
    return value ? value : "absent";
#endif
}
#ifndef _WIN32
volatile std::sig_atomic_t restored_count = 0;
extern "C" void previous_handler(int) {
    restored_count = 1;
}
#endif
} // namespace
int main(int argc, char **argv) {
    using namespace ghostwire::platform;
    using namespace std::chrono_literals;
    const auto parsed = arguments(argc, argv);
    if (!parsed || parsed.value().empty())
        return 90;
    const auto &args = parsed.value();
    const auto &mode = args.front();
    if (mode == "echo") {
        for (std::size_t i = 1; i < args.size(); ++i)
            std::cout << args[i].size() << ':' << args[i] << '\n';
        std::cout << "cwd=" << path_to_utf8(std::filesystem::current_path()) << '\n';
        std::cout << "env=" << test_environment() << '\n';
        std::cout << "stdin=" << (std::cin.get() == std::char_traits<char>::eof() ? "eof" : "data") << '\n';
        std::cerr << "child-stderr\n";
        return 23;
    }
    if (mode == "flood") {
        const std::string block(8192, 'X');
        for (unsigned i = 0; i < 256; ++i) {
            std::cout << block;
            std::cerr << block;
        }
        return 7;
    }
    if (mode == "sleep") {
        std::cout << "ready\n" << std::flush;
        std::this_thread::sleep_for(10s);
        return 0;
    }
    if (mode == "bridge") {
        auto bridge = ProcessStopBridge::install();
        if (!bridge || ProcessStopBridge::install())
            return 91;
        const auto owner = std::this_thread::get_id();
        bool on_owner = false;
        const auto token = bridge.value()->stop_token();
        std::stop_callback callback(token, [&] { on_owner = std::this_thread::get_id() == owner; });
        std::cout << "ready\n" << std::flush;
        const auto until = std::chrono::steady_clock::now() + 10s;
        while (!token.stop_requested() && std::chrono::steady_clock::now() < until) {
            if (!bridge.value()->dispatch())
                return 92;
            std::this_thread::sleep_for(5ms);
        }
        if (!on_owner || !bridge.value()->close() || !bridge.value()->close() || bridge.value()->dispatch())
            return 93;
        auto next = ProcessStopBridge::install();
        if (!next || next.value()->stop_token().stop_requested() || !next.value()->close())
            return 94;
        std::cout << "stopped-on-owner\n";
        return 0;
    }
    if (mode == "descendant" && args.size() == 2) {
#ifdef _WIN32
        auto executable = path_from_utf8(args[1]);
        if (!executable)
            return 95;
        std::wstring command = L"\"" + executable.value().native() + L"\" sleep";
        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        startup.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        PROCESS_INFORMATION child{};
        if (!CreateProcessW(executable.value().c_str(), command.data(), nullptr, nullptr, TRUE, 0, nullptr,
                            nullptr, &startup, &child))
            return 95;
        CloseHandle(child.hThread);
        CloseHandle(child.hProcess);
#else
        const auto child = fork();
        if (child < 0)
            return 95;
        if (child == 0) {
            // Deliberately retain the inherited pipes until the owned group dies.
            sleep(10);
            _exit(0);
        }
#endif
        std::cout << "descendant-started\n";
        return 0;
    }
#ifdef _WIN32
    if (mode == "handle" && args.size() == 5) {
        const auto value = static_cast<std::uintptr_t>(std::stoull(args[1]));
        BY_HANDLE_FILE_INFORMATION info{};
        const bool inherited = GetFileInformationByHandle(reinterpret_cast<HANDLE>(value), &info) &&
                               info.dwVolumeSerialNumber == std::stoul(args[2]) &&
                               info.nFileIndexHigh == std::stoul(args[3]) &&
                               info.nFileIndexLow == std::stoul(args[4]);
        return inherited ? 97 : 0;
    }
#else
    if (mode == "restoration") {
        struct sigaction previous{}, action{};
        action.sa_handler = previous_handler;
        sigemptyset(&action.sa_mask);
        if (sigaction(SIGINT, &action, &previous) != 0)
            return 96;
        auto bridge = ProcessStopBridge::install();
        if (!bridge || raise(SIGINT) != 0 || bridge.value()->stop_token().stop_requested())
            return 96;
        const auto event = bridge.value()->dispatch();
        if (!event || event.value() != ProcessStopEvent::interrupt ||
            !bridge.value()->stop_token().stop_requested())
            return 96;
        bridge.value().reset();
        if (raise(SIGINT) != 0 || restored_count != 1)
            return 96;
        if (sigaction(SIGINT, &previous, nullptr) != 0)
            return 96;
        auto next = ProcessStopBridge::install();
        if (!next || next.value()->dispatch().value() != ProcessStopEvent::none)
            return 96;
        return 0;
    }
    if (mode == "descriptor" && args.size() == 2) {
        const int descriptor = std::stoi(args[1]);
        return fcntl(descriptor, F_GETFD) < 0 && errno == EBADF ? 0 : 97;
    }
#endif
    return 98;
}
