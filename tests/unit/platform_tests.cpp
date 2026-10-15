#define DOCTEST_CONFIG_IMPLEMENT
#include "ghostwire/core/application.hpp"
#include "ghostwire/platform/arguments.hpp"
#include "ghostwire/platform/filesystem.hpp"
#include "ghostwire/platform/process.hpp"
#include "ghostwire/platform/process_stop.hpp"
#include "ghostwire/platform/secure_storage.hpp"
#include <array>
#include <atomic>
#include <cstdlib>
#include <doctest/doctest.h>
#include <fstream>
#include <iostream>
#include <latch>
#include <sstream>
#include <thread>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace ghostwire;
using namespace ghostwire::platform;
using namespace std::chrono_literals;
namespace fs = std::filesystem;
namespace {
fs::path fixture;
class TemporaryDirectory {
  public:
    TemporaryDirectory() {
        const auto base = fs::canonical(fs::temp_directory_path());
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (unsigned i = 0; i < 100; ++i) {
            path = base / path_from_utf8("ghostwire-platform-\xc3\xb6 space-" + std::to_string(stamp) + "-" +
                                         std::to_string(i))
                              .value();
            if (fs::create_directory(path))
                return;
        }
        throw std::runtime_error("Cannot create platform test directory.");
    }
    ~TemporaryDirectory() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
    fs::path path;
};
ProcessOptions options(std::string mode) {
    ProcessOptions result;
    result.executable = fixture;
    result.working_directory = fixture.parent_path();
    result.arguments = {std::move(mode)};
    result.output = ProcessOutput::capture;
#ifndef _WIN32
    // An explicitly requested sanitizer policy must reach instrumented children
    // even while testing replacement environments. No policy is set by default.
    for (const auto *name : {"ASAN_OPTIONS", "UBSAN_OPTIONS", "TSAN_OPTIONS"})
        if (const auto *value = std::getenv(name))
            result.variables.push_back({name, value});
#endif
    return result;
}
bool ready(Process &process) {
    const auto until = std::chrono::steady_clock::now() + 3s;
    while (std::chrono::steady_clock::now() < until) {
        const auto state = process.snapshot();
        if (state.output.bytes.find("ready\n") != std::string::npos ||
            state.output.bytes.find("ready\r\n") != std::string::npos)
            return true;
        if (!state.running)
            return false;
        std::this_thread::sleep_for(5ms); // Bounded fixture handshake, no spin loop.
    }
    return false;
}
bool no_owned_child(std::uint64_t identifier) {
#ifdef _WIN32
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(identifier));
    if (!process)
        return GetLastError() == ERROR_INVALID_PARAMETER;
    const bool exited = WaitForSingleObject(process, 0) == WAIT_OBJECT_0;
    CloseHandle(process);
    return exited;
#else
    int status = 0;
    return waitpid(static_cast<pid_t>(identifier), &status, WNOHANG) == -1 && errno == ECHILD &&
           kill(static_cast<pid_t>(identifier), 0) == -1 && errno == ESRCH;
#endif
}
} // namespace

TEST_CASE("owned process preserves Unicode executable cwd argv quotes empty values and nonzero exit") {
    TemporaryDirectory temp;
    const auto executable = temp.path / path_from_utf8("child \xc3\xb6.exe").value();
    fs::copy_file(fixture, executable);
    auto specification = options("echo");
    specification.executable = executable;
    specification.working_directory = temp.path;
    const std::vector<std::string> values = {"",
                                             "spaces here",
                                             "\xc3\xb6",
                                             "quote\"inside",
                                             "tail\\",
                                             "slashes\\\\\"quoted",
                                             "& | ; $(touch must-not-exist) %TEMP% !"};
    specification.arguments.insert(specification.arguments.end(), values.begin(), values.end());
    specification.variables.push_back({"GHOSTWIRE_CHILD_VALUE", "value \xc3\xb6 = text"});
    auto child = Process::launch(specification);
    REQUIRE(child);
    const auto exit = child.value()->wait_for(3s);
    REQUIRE(exit);
    CHECK(exit.value() == ProcessExit{23, 0});
    const auto state = child.value()->snapshot();
    CHECK_FALSE(state.running);
    CHECK_FALSE(state.output.truncated);
    CHECK_FALSE(state.errors.truncated);
    for (const auto &value : values)
        CHECK(state.output.bytes.find(std::to_string(value.size()) + ':' + value) != std::string::npos);
    CHECK(state.output.bytes.find("cwd=" + path_to_utf8(temp.path)) != std::string::npos);
    CHECK(state.output.bytes.find("env=value \xc3\xb6 = text") != std::string::npos);
    CHECK(state.output.bytes.find("stdin=eof") != std::string::npos);
    CHECK(state.errors.bytes.find("child-stderr") != std::string::npos);
    CHECK_FALSE(fs::exists(temp.path / "must-not-exist"));
    CHECK(child.value()->wait_for(0ms).value() == exit.value());
    CHECK(child.value()->request_stop());
    CHECK(child.value()->terminate());
    CHECK(no_owned_child(state.identifier));
}
TEST_CASE("completed children without descendants preserve exit status and repeat-safe cleanup") {
    for (const auto output : {ProcessOutput::capture, ProcessOutput::discard}) {
        for (unsigned attempt = 0; attempt < 4; ++attempt) {
            auto specification = options("echo");
            specification.output = output;
            auto child = Process::launch(specification);
            REQUIRE(child);
            const auto exit = child.value()->wait_for(3s);
            const int reason = exit ? 0 : static_cast<int>(exit.error().reason);
            CAPTURE(reason);
            REQUIRE(exit);
            CHECK(exit.value() == ProcessExit{23, 0});
            const auto state = child.value()->snapshot();
            CHECK_FALSE(state.failure);
            CHECK_FALSE(state.running);
            CHECK(state.exit == exit.value());
            CHECK(child.value()->request_stop());
            CHECK(child.value()->terminate());
            const auto repeated = child.value()->wait_for(0ms);
            REQUIRE(repeated);
            CHECK(repeated.value() == exit.value());
            CHECK(no_owned_child(state.identifier));
        }
    }
}
TEST_CASE("process launch rejects invalid paths encoding options and duplicate environment names") {
    const auto check = [](const ProcessOptions &specification) {
        auto child = Process::launch(specification);
        REQUIRE_FALSE(child);
        CHECK(child.error().reason == core::ErrorReason::invalid_process_options);
        CHECK(child.error().message.find("private-marker") == std::string::npos);
    };
    auto bad = options("echo");
    bad.executable = "relative-private-marker";
    check(bad);
    bad = options("echo");
    bad.working_directory.clear();
    check(bad);
    bad = options("echo");
    bad.arguments.push_back("private-marker-\xf6");
    check(bad);
    bad = options("echo");
    bad.arguments.push_back(std::string("private-marker\0x", 16));
    check(bad);
    bad = options("echo");
    bad.arguments.resize(129, "x");
    check(bad);
    bad = options("echo");
    bad.arguments.push_back(std::string(4097, 'x'));
    check(bad);
    bad = options("echo");
    bad.arguments.resize(17, std::string(4096, 'x'));
    check(bad);
    bad = options("echo");
    bad.capture_bytes_per_stream = 1024 * 1024 + 1;
    check(bad);
    bad = options("echo");
    bad.variables = {{"bad=name", "secret"}};
    check(bad);
    bad = options("echo");
    bad.variables = {{"Name", "a"}, {"NAME", "b"}};
    check(bad);
    bad = options("echo");
    bad.variables = {{"NAME", "private-marker-\xc0\xaf"}};
    check(bad);
    bad = options("echo");
    bad.environment = static_cast<ProcessEnvironment>(99);
    check(bad);
    bad = options("echo");
    bad.output = static_cast<ProcessOutput>(99);
    check(bad);
}
TEST_CASE("missing executable cwd and non-executable file fail without private path disclosure") {
    TemporaryDirectory temp;
    for (unsigned mode = 0; mode < 3; ++mode) {
        auto specification = options("echo");
        if (mode == 0)
            specification.executable = temp.path / "private-marker-missing";
        if (mode == 1)
            specification.working_directory = temp.path / "private-marker-cwd";
        if (mode == 2) {
            specification.executable = temp.path / "private-marker-invalid.exe";
            std::ofstream file(specification.executable);
            file << "not an executable";
        }
        auto child = Process::launch(specification);
        REQUIRE_FALSE(child);
        CHECK(child.error().reason == core::ErrorReason::process_launch_failed);
        CHECK(child.error().message.find("private-marker") == std::string::npos);
    }
}
TEST_CASE("concurrent stdout stderr flood is drained after capture budget without deadlock") {
    auto specification = options("flood");
    specification.capture_bytes_per_stream = 1024;
    auto child = Process::launch(specification);
    REQUIRE(child);
    const auto exit = child.value()->wait_for(5s);
    REQUIRE(exit);
    CHECK(exit.value().code == 7);
    const auto snapshot = child.value()->snapshot();
    CHECK(snapshot.output.bytes == std::string(1024, 'X'));
    CHECK(snapshot.errors.bytes == std::string(1024, 'X'));
    CHECK(snapshot.output.truncated);
    CHECK(snapshot.errors.truncated);
}
TEST_CASE("discard and zero capture modes do not retain output or block child") {
    for (const auto mode : {ProcessOutput::discard, ProcessOutput::capture}) {
        auto specification = options("echo");
        specification.output = mode;
        specification.capture_bytes_per_stream = 0;
        auto child = Process::launch(specification);
        REQUIRE(child);
        REQUIRE(child.value()->wait_for(3s));
        const auto state = child.value()->snapshot();
        CHECK(state.output.bytes.empty());
        CHECK(state.errors.bytes.empty());
        CHECK(state.output.truncated == (mode == ProcessOutput::capture));
    }
}
TEST_CASE("wait timeout retains child ownership and explicit termination reaps exactly that child") {
    auto child = Process::launch(options("sleep"));
    auto other = Process::launch(options("sleep"));
    REQUIRE(child);
    REQUIRE(other);
    REQUIRE(ready(*child.value()));
    REQUIRE(ready(*other.value()));
    const auto started = std::chrono::steady_clock::now();
    const auto waited = child.value()->wait_for(15ms);
    REQUIRE_FALSE(waited);
    CHECK(waited.error().reason == core::ErrorReason::deadline_exceeded);
    CHECK(std::chrono::steady_clock::now() - started < 2s);
    CHECK(child.value()->snapshot().running);
    CHECK_FALSE(child.value()->wait_for(-1ms));
    CHECK_FALSE(child.value()->wait_for(60001ms));
    CHECK(child.value()->terminate());
    CHECK(child.value()->terminate());
    REQUIRE(child.value()->wait_for(3s));
    CHECK(no_owned_child(child.value()->snapshot().identifier));
    CHECK(other.value()->snapshot().running);
    CHECK(other.value()->terminate());
    REQUIRE(other.value()->wait_for(3s));
}
TEST_CASE("standard cancellation interrupts waiting and pre-cancelled launch has no child") {
    std::stop_source stopped;
    stopped.request_stop();
    auto rejected = Process::launch(options("sleep"), stopped.get_token());
    REQUIRE_FALSE(rejected);
    CHECK(rejected.error().reason == core::ErrorReason::cancelled);
    auto child = Process::launch(options("sleep"));
    REQUIRE(child);
    REQUIRE(ready(*child.value()));
    std::stop_source stop;
    std::jthread canceller([&] {
        std::this_thread::sleep_for(20ms);
        stop.request_stop();
    });
    const auto waited = child.value()->wait_for(3s, stop.get_token());
    REQUIRE_FALSE(waited);
    CHECK(waited.error().reason == core::ErrorReason::cancelled);
    CHECK(child.value()->snapshot().running);
    CHECK(child.value()->terminate());
    REQUIRE(child.value()->wait_for(3s));
}
TEST_CASE("RAII destroys an unfinished child and joins its monitor") {
    std::uint64_t id = 0;
    {
        auto child = Process::launch(options("sleep"));
        REQUIRE(child);
        REQUIRE(ready(*child.value()));
        id = child.value()->snapshot().identifier;
    }
    CHECK(no_owned_child(id));
}
TEST_CASE("concurrent waiters observe one owned exit and serialize monitor joining") {
    auto child = Process::launch(options("sleep"));
    REQUIRE(child);
    REQUIRE(ready(*child.value()));
    std::latch entered(2);
    std::array<std::optional<core::Result<ProcessExit>>, 2> results;
    std::jthread first([&] {
        entered.count_down();
        results[0].emplace(child.value()->wait_for(3s));
    });
    std::jthread second([&] {
        entered.count_down();
        results[1].emplace(child.value()->wait_for(3s));
    });
    entered.wait();
    CHECK(child.value()->terminate());
    first.join();
    second.join();
    REQUIRE(results[0]);
    REQUIRE(results[1]);
    REQUIRE(*results[0]);
    REQUIRE(*results[1]);
    CHECK(results[0]->value() == results[1]->value());
    CHECK(no_owned_child(child.value()->snapshot().identifier));
}
TEST_CASE("native stop delivery requests cancellation on child owner and supports reinstall") {
    auto child = Process::launch(options("bridge"));
    REQUIRE(child);
    REQUIRE(ready(*child.value()));
    REQUIRE(child.value()->request_stop());
    CHECK(child.value()->request_stop());
    const auto exit = child.value()->wait_for(3s);
    REQUIRE(exit);
    CHECK(exit.value() == ProcessExit{});
    CHECK(child.value()->snapshot().output.bytes.find("stopped-on-owner") != std::string::npos);
}
TEST_CASE("owned group or job closes descendant output writers after leader exits") {
    auto specification = options("descendant");
    specification.arguments.push_back(path_to_utf8(fixture));
    auto child = Process::launch(specification);
    REQUIRE(child);
    const auto exit = child.value()->wait_for(3s);
    REQUIRE(exit);
    CHECK(exit.value() == ProcessExit{});
    CHECK(child.value()->snapshot().output.bytes.find("descendant-started") != std::string::npos);
    CHECK_FALSE(child.value()->snapshot().output.truncated);
}
TEST_CASE("bridge has explicit ownership repeated close wrong-thread errors and live tokens") {
    auto bridge = ProcessStopBridge::install();
    REQUIRE(bridge);
    const auto token = bridge.value()->stop_token();
    auto duplicate = ProcessStopBridge::install();
    REQUIRE_FALSE(duplicate);
    CHECK(duplicate.error().reason == core::ErrorReason::invalid_state);
    std::atomic<bool> rejected{false};
    std::jthread thread([&] { rejected = !bridge.value()->dispatch() && !bridge.value()->close(); });
    thread.join();
    CHECK(rejected.load());
    CHECK(bridge.value()->dispatch().value() == ProcessStopEvent::none);
    CHECK(bridge.value()->close());
    CHECK(bridge.value()->close());
    CHECK_FALSE(bridge.value()->dispatch());
    bridge.value().reset();
    CHECK_FALSE(token.stop_requested());
    auto next = ProcessStopBridge::install();
    REQUIRE(next);
    CHECK(next.value()->close());
}
TEST_CASE("secure storage capability states never authorize persistence") {
    const auto disabled = probe_secure_storage(SecureStorageProbe::disabled);
    REQUIRE(disabled);
    CHECK(disabled.value().api_availability == CapabilityAvailability::unavailable);
    CHECK(disabled.value().reason == SecureStorageReason::disabled_by_policy);
    CHECK_FALSE(disabled.value().persistence_ready);
    const auto native = probe_secure_storage();
    REQUIRE(native);
    CHECK_FALSE(native.value().persistence_ready);
#if defined(_WIN32) || defined(__APPLE__)
    CHECK(native.value().supported);
    CHECK(native.value().api_availability == CapabilityAvailability::available);
    CHECK(native.value().reason == SecureStorageReason::api_present);
#else
    CHECK_FALSE(native.value().supported);
    CHECK(native.value().api_availability == CapabilityAvailability::unsupported);
    CHECK(native.value().reason == SecureStorageReason::adapter_not_built);
#endif
    const auto invalid = probe_secure_storage(static_cast<SecureStorageProbe>(999));
    REQUIRE_FALSE(invalid);
    CHECK(invalid.error().reason == core::ErrorReason::invalid_state);
    std::stop_source stop;
    stop.request_stop();
    const auto cancelled = probe_secure_storage(SecureStorageProbe::native_api, stop.get_token());
    REQUIRE_FALSE(cancelled);
    CHECK(cancelled.error().reason == core::ErrorReason::cancelled);
}
TEST_CASE("replacement inheritance and overrides never change the parent environment") {
    struct EnvironmentGuard {
#ifdef _WIN32
        std::wstring previous;
        bool existed = false;
        EnvironmentGuard() {
            SetLastError(ERROR_SUCCESS);
            const auto size = GetEnvironmentVariableW(L"GHOSTWIRE_CHILD_VALUE", nullptr, 0);
            existed = size != 0 || GetLastError() == ERROR_SUCCESS;
            if (size) {
                previous.resize(size);
                const auto copied = GetEnvironmentVariableW(L"GHOSTWIRE_CHILD_VALUE", previous.data(), size);
                previous.resize(copied);
            }
            if (!SetEnvironmentVariableW(L"GHOSTWIRE_CHILD_VALUE", L"parent \u00f6"))
                throw std::runtime_error("Cannot set fixture environment.");
        }
        ~EnvironmentGuard() {
            SetEnvironmentVariableW(L"GHOSTWIRE_CHILD_VALUE", existed ? previous.c_str() : nullptr);
        }
#else
        std::optional<std::string> previous;
        EnvironmentGuard() {
            if (const auto *value = std::getenv("GHOSTWIRE_CHILD_VALUE"))
                previous = value;
            if (setenv("GHOSTWIRE_CHILD_VALUE", "parent \xc3\xb6", 1) != 0)
                throw std::runtime_error("Cannot set fixture environment.");
        }
        ~EnvironmentGuard() {
            if (previous)
                setenv("GHOSTWIRE_CHILD_VALUE", previous->c_str(), 1);
            else
                unsetenv("GHOSTWIRE_CHILD_VALUE");
        }
#endif
    } environment;
    for (unsigned mode = 0; mode < 4; ++mode) {
        auto specification = options("echo");
        specification.environment = mode == 0 ? ProcessEnvironment::replace : ProcessEnvironment::inherit;
        if (mode == 2)
            specification.variables.push_back({"GHOSTWIRE_CHILD_VALUE", "override \xc3\xb6"});
        auto child = Process::launch(specification);
        REQUIRE(child);
        REQUIRE(child.value()->wait_for(3s));
        const auto expected = mode == 0 ? "absent" : mode == 2 ? "override \xc3\xb6" : "parent \xc3\xb6";
        CHECK(child.value()->snapshot().output.bytes.find(std::string("env=") + expected) !=
              std::string::npos);
    }
}
TEST_CASE("an application service owns a real child through cancellation and reverse cleanup") {
    class ChildService final : public core::Service {
      public:
        std::unique_ptr<Process> child;
        std::stop_token application_stop;
        core::Result<void> start(const core::ApplicationContext &context) override {
            application_stop = context.stop_token();
            auto launched = Process::launch(options("bridge"), application_stop);
            if (!launched)
                return launched.error();
            child = std::move(launched).value();
            if (!ready(*child))
                return core::Error{core::ErrorCode::internal, "Child handshake failed."};
            return {};
        }
        core::Result<void> stop(const core::ShutdownContext &context) override {
            if (!child)
                return {};
            auto signaled = child->request_stop();
            if (!signaled)
                return signaled.error();
            const auto waited =
                child->wait_for(std::chrono::duration_cast<std::chrono::milliseconds>(context.remaining()));
            if (!waited)
                return waited.error();
            return {};
        }
    };
    TemporaryDirectory temp;
    core::Config config;
    config.data_root = temp.path;
    const auto paths = core::resolve_paths(core::Role::cli, temp.path, {});
    REQUIRE(paths);
    std::ostringstream logs;
    core::Logger logger(logs, core::LogLevel::off);
    core::Application app(core::Role::cli, config, paths.value(), logger);
    auto service = std::make_unique<ChildService>();
    auto *observed = service.get();
    const std::array dependencies{core::application_paths_service};
    REQUIRE(app.add_service({2}, std::move(service), dependencies));
    REQUIRE(app.start());
    app.request_stop();
    CHECK(observed->application_stop.stop_requested());
    REQUIRE(app.shutdown(3000ms));
    CHECK(app.shutdown_report().complete);
    CHECK(no_owned_child(observed->child->snapshot().identifier));
    CHECK(observed->child->snapshot().exit == ProcessExit{});
}
#ifdef _WIN32
TEST_CASE("Windows handle list excludes unrelated inheritable file handles") {
    TemporaryDirectory temp;
    SECURITY_ATTRIBUTES attributes{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE handle = CreateFileW((temp.path / "private-handle").c_str(), GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, &attributes, CREATE_NEW, 0, nullptr);
    REQUIRE(handle != INVALID_HANDLE_VALUE);
    struct Close {
        HANDLE value;
        ~Close() { CloseHandle(value); }
    } guard{handle};
    BY_HANDLE_FILE_INFORMATION identity{};
    REQUIRE(GetFileInformationByHandle(handle, &identity));
    auto specification = options("handle");
    specification.arguments.insert(specification.arguments.end(),
                                   {std::to_string(reinterpret_cast<std::uintptr_t>(handle)),
                                    std::to_string(identity.dwVolumeSerialNumber),
                                    std::to_string(identity.nFileIndexHigh),
                                    std::to_string(identity.nFileIndexLow)});
    auto child = Process::launch(specification);
    REQUIRE(child);
    const auto exit = child.value()->wait_for(3s);
    REQUIRE(exit);
    CHECK(exit.value() == ProcessExit{});
}
#endif
#ifndef _WIN32
TEST_CASE("POSIX restores earlier signal handler and does not deliver stale events after reinstall") {
    auto child = Process::launch(options("restoration"));
    REQUIRE(child);
    const auto exit = child.value()->wait_for(3s);
    REQUIRE(exit);
    CHECK(exit.value() == ProcessExit{});
}
TEST_CASE("POSIX owned launch closes foreign descriptors even when caller omitted CLOEXEC") {
    TemporaryDirectory temp;
    const int descriptor = open((temp.path / "private-descriptor").c_str(), O_CREAT | O_RDWR, 0600);
    REQUIRE(descriptor >= 0);
    struct Close {
        int fd;
        ~Close() { close(fd); }
    } guard{descriptor};
    auto specification = options("descriptor");
    specification.arguments.push_back(std::to_string(descriptor));
    auto child = Process::launch(specification);
    REQUIRE(child);
    const auto exit = child.value()->wait_for(3s);
    REQUIRE(exit);
    CHECK(exit.value() == ProcessExit{});
}
TEST_CASE("POSIX rejects automatic external child reaping before launch") {
    struct sigaction previous{}, ignored{};
    ignored.sa_handler = SIG_IGN;
    sigemptyset(&ignored.sa_mask);
    REQUIRE(sigaction(SIGCHLD, &ignored, &previous) == 0);
    struct Restore {
        struct sigaction old;
        ~Restore() { sigaction(SIGCHLD, &old, nullptr); }
    } guard{previous};
    auto child = Process::launch(options("sleep"));
    REQUIRE_FALSE(child);
    CHECK(child.error().reason == core::ErrorReason::invalid_state);
}
#endif
int main(int argc, char **argv) {
    const auto args = platform::arguments(argc, argv);
    if (!args || args.value().size() != 1)
        return 2;
    const auto decoded = path_from_utf8(args.value().front());
    if (!decoded || !decoded.value().is_absolute())
        return 2;
    fixture = decoded.value();
#ifdef _WIN32
    // CTest may itself run without a console (for example in a CI service).
    // The CTRL_BREAK tests need a shared console; own one only when absent.
    struct TestConsole {
        bool owned = false;
        ~TestConsole() {
            if (owned)
                FreeConsole();
        }
    } console;
    if (GetConsoleCP() == 0) {
        if (!AllocConsole()) {
            std::cerr << "Cannot create the console required for native stop tests.\n";
            return 1;
        }
        console.owned = true;
    }
#endif
    doctest::Context context;
    context.setOption("no-colors", true);
    return context.run();
}
