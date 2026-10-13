#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include "ghostwire/platform/process.hpp"
#include "ghostwire/platform/filesystem.hpp"
#include <algorithm>
#include <array>
#include <condition_variable>
#include <map>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>
#include <utility>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
#endif

namespace ghostwire::platform {
using core::Error;
using core::ErrorCode;
using core::ErrorReason;
using core::Result;
namespace {
constexpr std::size_t max_capture = 1024 * 1024, max_input = 64 * 1024;
Error process_error(ErrorReason reason) {
    return {ErrorCode::internal, "Owned process operation failed.", reason};
}
Result<void> validate(const ProcessOptions &options) {
    const auto invalid = [] {
        return Error{ErrorCode::usage, "Invalid process options.", ErrorReason::invalid_process_options};
    };
    if (!options.executable.is_absolute() || !options.working_directory.is_absolute() ||
        options.arguments.size() > 128 || options.variables.size() > 64 ||
        options.capture_bytes_per_stream > max_capture ||
        (options.environment != ProcessEnvironment::inherit &&
         options.environment != ProcessEnvironment::replace) ||
        (options.output != ProcessOutput::capture && options.output != ProcessOutput::discard))
        return invalid();
    std::size_t bytes = 0;
    const auto text_valid = [&](const std::string &value) {
        if (value.size() > 4096 || value.size() > max_input - bytes || !path_from_utf8(value))
            return false;
        bytes += value.size();
        return true;
    };
    if (!text_valid(path_to_utf8(options.executable)) || !text_valid(path_to_utf8(options.working_directory)))
        return invalid();
    for (const auto &arg : options.arguments)
        if (!text_valid(arg))
            return invalid();
    std::set<std::string> names;
    for (const auto &variable : options.variables) {
        if (variable.name.empty() || variable.name.size() > 128 || !text_valid(variable.value))
            return invalid();
        std::string key;
        for (const char c : variable.name) {
            if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' ||
                  (!key.empty() && c >= '0' && c <= '9')))
                return invalid();
            key += (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
        }
        if (!names.insert(key).second)
            return invalid();
    }
    return {};
}
#ifdef _WIN32
struct Handle {
    HANDLE value = nullptr;
    Handle() = default;
    explicit Handle(HANDLE handle) : value(handle) {}
    ~Handle() { reset(); }
    Handle(const Handle &) = delete;
    Handle &operator=(const Handle &) = delete;
    void reset(HANDLE handle = nullptr) noexcept {
        if (value && value != INVALID_HANDLE_VALUE)
            CloseHandle(value);
        value = handle;
    }
    explicit operator bool() const { return value && value != INVALID_HANDLE_VALUE; }
};
std::wstring wide(const std::string &text) {
    if (text.empty())
        return {};
    const auto size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                                          static_cast<int>(text.size()), nullptr, 0);
    if (size <= 0)
        throw std::runtime_error("Invalid process text.");
    std::wstring result(static_cast<std::size_t>(size), L'\0');
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()),
                            result.data(), size) != size)
        throw std::runtime_error("Invalid process text.");
    return result;
}
// Microsoft CRT/CommandLineToArgvW quoting. No cmd.exe interpretation occurs.
std::wstring quote(const std::wstring &value) {
    std::wstring result(1, L'"');
    std::size_t slashes = 0;
    for (const wchar_t c : value) {
        if (c == L'\\') {
            ++slashes;
            continue;
        }
        result.append(c == L'"' ? slashes * 2 + 1 : slashes, L'\\');
        slashes = 0;
        result += c;
    }
    result.append(slashes * 2, L'\\');
    result += L'"';
    return result;
}
struct EnvironmentLess {
    bool operator()(const std::wstring &a, const std::wstring &b) const {
        return CompareStringOrdinal(a.c_str(), -1, b.c_str(), -1, TRUE) == CSTR_LESS_THAN;
    }
};
Result<std::vector<wchar_t>> environment_block(const ProcessOptions &options) {
    std::map<std::wstring, std::wstring, EnvironmentLess> values;
    if (options.environment == ProcessEnvironment::inherit) {
        auto *raw = GetEnvironmentStringsW();
        if (!raw)
            return process_error(ErrorReason::native_failure);
        struct FreeEnvironment {
            void operator()(wchar_t *p) const { FreeEnvironmentStringsW(p); }
        };
        std::unique_ptr<wchar_t, FreeEnvironment> owned(raw);
        std::size_t total = 0;
        for (auto *entry = raw; *entry;) {
            const std::wstring text(entry);
            total += text.size() + 1;
            if (total > max_capture)
                return process_error(ErrorReason::resource_exhausted);
            const auto separator = text.find(L'=', 1); // Keep native =C: drive entries.
            if (separator != std::wstring::npos)
                values[text.substr(0, separator)] = text.substr(separator + 1);
            entry += text.size() + 1;
        }
    }
    for (const auto &variable : options.variables)
        values[wide(variable.name)] = wide(variable.value);
    std::vector<wchar_t> result;
    for (const auto &[name, value] : values) {
        result.insert(result.end(), name.begin(), name.end());
        result.push_back(L'=');
        result.insert(result.end(), value.begin(), value.end());
        result.push_back(L'\0');
    }
    if (result.empty())
        result.push_back(L'\0');
    result.push_back(L'\0');
    return result;
}
#else
struct Descriptor {
    int value = -1;
    Descriptor() = default;
    explicit Descriptor(int descriptor) : value(descriptor) {}
    ~Descriptor() { reset(); }
    Descriptor(const Descriptor &) = delete;
    Descriptor &operator=(const Descriptor &) = delete;
    void reset(int descriptor = -1) noexcept {
        if (value >= 0)
            ::close(value); // Never retry close: the fd may already have been reused.
        value = descriptor;
    }
    bool above_stdio() {
        if (value >= 3)
            return true;
        if (value < 0)
            return false;
        const int copy = fcntl(value, F_DUPFD_CLOEXEC, 3);
        if (copy < 0)
            return false;
        reset(copy);
        return true;
    }
};
bool pipe_pair(Descriptor &read_end, Descriptor &write_end) {
    int values[2];
#ifdef __linux__
    if (pipe2(values, O_CLOEXEC) != 0)
        return false;
#else
    if (pipe(values) != 0)
        return false;
#endif
    read_end.reset(values[0]);
    write_end.reset(values[1]);
    if (!read_end.above_stdio() || !write_end.above_stdio() ||
        fcntl(read_end.value, F_SETFD, FD_CLOEXEC) != 0 || fcntl(write_end.value, F_SETFD, FD_CLOEXEC) != 0)
        return false;
    const int flags = fcntl(read_end.value, F_GETFL);
    return flags >= 0 && fcntl(read_end.value, F_SETFL, flags | O_NONBLOCK) == 0;
}
Result<std::vector<std::string>> environment_block(const ProcessOptions &options) {
    std::map<std::string, std::string> values;
    std::size_t total = 0;
    if (options.environment == ProcessEnvironment::inherit)
        for (auto **entry = environ; entry && *entry; ++entry) {
            std::string text(*entry);
            total += text.size() + 1;
            if (total > max_capture)
                return process_error(ErrorReason::resource_exhausted);
            const auto separator = text.find('=');
            if (separator != std::string::npos)
                values[text.substr(0, separator)] = text.substr(separator + 1);
        }
    for (const auto &variable : options.variables)
        values[variable.name] = variable.value;
    std::vector<std::string> result;
    for (const auto &[name, value] : values)
        result.push_back(name + '=' + value);
    return result;
}
#endif
} // namespace

struct Process::Impl {
    mutable std::mutex mutex;
    std::mutex join_mutex;
    std::condition_variable_any changed;
    ProcessSnapshot state;
    std::size_t limit = 0;
    bool graceful_requested = false, forced = false, native_live = false;
#ifdef _WIN32
    Handle process, job, output, errors;
    bool job_assigned = false;
#else
    pid_t child = -1;
    Descriptor output, errors;
#endif
    std::jthread monitor; // Must join before any of the above members die.

    Result<void> spawn(const ProcessOptions &options) {
        limit = options.capture_bytes_per_stream;
        auto environment = environment_block(options);
        if (!environment)
            return environment.error();
#ifdef _WIN32
        SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
        Handle input(CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security,
                                 OPEN_EXISTING, 0, nullptr));
        Handle write_out, write_err;
        if (!input)
            return process_error(ErrorReason::process_launch_failed);
        if (options.output == ProcessOutput::capture) {
            // A bounded 64 KiB buffer hint avoids throttling a busy child to the
            // small default pipe buffer at each idle observation interval.
            if (!CreatePipe(&output.value, &write_out.value, &security, static_cast<DWORD>(max_input)) ||
                !CreatePipe(&errors.value, &write_err.value, &security, static_cast<DWORD>(max_input)) ||
                !SetHandleInformation(output.value, HANDLE_FLAG_INHERIT, 0) ||
                !SetHandleInformation(errors.value, HANDLE_FLAG_INHERIT, 0))
                return process_error(ErrorReason::process_launch_failed);
        } else {
            write_out.reset(CreateFileW(L"NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &security,
                                        OPEN_EXISTING, 0, nullptr));
            write_err.reset(CreateFileW(L"NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &security,
                                        OPEN_EXISTING, 0, nullptr));
            if (!write_out || !write_err)
                return process_error(ErrorReason::process_launch_failed);
        }
        SIZE_T attribute_bytes = 0;
        InitializeProcThreadAttributeList(nullptr, 1, 0, &attribute_bytes);
        if (!attribute_bytes)
            return process_error(ErrorReason::process_launch_failed);
        std::vector<std::byte> attributes(attribute_bytes);
        auto *attribute_list = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributes.data());
        if (!InitializeProcThreadAttributeList(attribute_list, 1, 0, &attribute_bytes))
            return process_error(ErrorReason::process_launch_failed);
        struct DeleteAttributes {
            LPPROC_THREAD_ATTRIBUTE_LIST value;
            ~DeleteAttributes() { DeleteProcThreadAttributeList(value); }
        } cleanup{attribute_list};
        HANDLE inherited[] = {input.value, write_out.value, write_err.value};
        if (!UpdateProcThreadAttribute(attribute_list, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inherited,
                                       sizeof(inherited), nullptr, nullptr))
            return process_error(ErrorReason::process_launch_failed);
        STARTUPINFOEXW startup{};
        startup.StartupInfo.cb = sizeof(startup);
        startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
        startup.StartupInfo.hStdInput = input.value;
        startup.StartupInfo.hStdOutput = write_out.value;
        startup.StartupInfo.hStdError = write_err.value;
        startup.lpAttributeList = attribute_list;
        job.reset(CreateJobObjectW(nullptr, nullptr));
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if (!job ||
            !SetInformationJobObject(job.value, JobObjectExtendedLimitInformation, &limits, sizeof(limits)))
            return process_error(ErrorReason::process_launch_failed);
        std::wstring command = quote(options.executable.native());
        for (const auto &arg : options.arguments)
            command += L' ' + quote(wide(arg));
        if (command.size() >= 32767)
            return Error{ErrorCode::usage, "Process command exceeds native limit.",
                         ErrorReason::invalid_process_options};
        PROCESS_INFORMATION info{};
        if (!CreateProcessW(options.executable.c_str(), command.data(), nullptr, nullptr, TRUE,
                            CREATE_SUSPENDED | CREATE_NEW_PROCESS_GROUP | CREATE_UNICODE_ENVIRONMENT |
                                EXTENDED_STARTUPINFO_PRESENT,
                            environment.value().data(), options.working_directory.c_str(),
                            &startup.StartupInfo, &info))
            return process_error(ErrorReason::process_launch_failed);
        process.reset(info.hProcess);
        Handle primary_thread(info.hThread);
        state.identifier = info.dwProcessId;
        state.running = true;
        native_live = true;
        if (!AssignProcessToJobObject(job.value, process.value))
            return process_error(ErrorReason::process_launch_failed);
        job_assigned = true;
        if (ResumeThread(primary_thread.value) == static_cast<DWORD>(-1))
            return process_error(ErrorReason::process_launch_failed);
#else
        struct sigaction child_action{};
        if (sigaction(SIGCHLD, nullptr, &child_action) != 0 || child_action.sa_handler == SIG_IGN ||
            (child_action.sa_flags & SA_NOCLDWAIT) != 0)
            return process_error(ErrorReason::invalid_state);
        Descriptor input(open("/dev/null", O_RDWR | O_CLOEXEC)), write_out, write_err;
        if (!input.above_stdio())
            return process_error(ErrorReason::process_launch_failed);
        if (options.output == ProcessOutput::capture &&
            (!pipe_pair(output, write_out) || !pipe_pair(errors, write_err)))
            return process_error(ErrorReason::process_launch_failed);
        posix_spawn_file_actions_t actions;
        posix_spawnattr_t attributes;
        if (posix_spawn_file_actions_init(&actions) != 0)
            return process_error(ErrorReason::process_launch_failed);
        struct ActionsGuard {
            posix_spawn_file_actions_t *value;
            ~ActionsGuard() { posix_spawn_file_actions_destroy(value); }
        } actions_guard{&actions};
        if (posix_spawnattr_init(&attributes) != 0)
            return process_error(ErrorReason::process_launch_failed);
        struct AttributesGuard {
            posix_spawnattr_t *value;
            ~AttributesGuard() { posix_spawnattr_destroy(value); }
        } attributes_guard{&attributes};
        const int stdout_fd = options.output == ProcessOutput::capture ? write_out.value : input.value;
        const int stderr_fd = options.output == ProcessOutput::capture ? write_err.value : input.value;
        if (posix_spawn_file_actions_adddup2(&actions, input.value, STDIN_FILENO) != 0 ||
            posix_spawn_file_actions_adddup2(&actions, stdout_fd, STDOUT_FILENO) != 0 ||
            posix_spawn_file_actions_adddup2(&actions, stderr_fd, STDERR_FILENO) != 0 ||
            GW_SPAWN_CHDIR(&actions, options.working_directory.c_str()) != 0)
            return process_error(ErrorReason::process_launch_failed);
        short flags = POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF;
#ifdef __APPLE__
        flags = static_cast<short>(flags | POSIX_SPAWN_CLOEXEC_DEFAULT);
#else
        if (posix_spawn_file_actions_addclosefrom_np(&actions, 3) != 0)
            return process_error(ErrorReason::process_launch_failed);
#endif
        sigset_t empty, defaults;
        sigemptyset(&empty);
        sigemptyset(&defaults);
        for (const int signal : {SIGINT, SIGTERM, SIGPIPE, SIGQUIT})
            sigaddset(&defaults, signal);
        if (posix_spawnattr_setflags(&attributes, flags) != 0 ||
            posix_spawnattr_setpgroup(&attributes, 0) != 0 ||
            posix_spawnattr_setsigmask(&attributes, &empty) != 0 ||
            posix_spawnattr_setsigdefault(&attributes, &defaults) != 0)
            return process_error(ErrorReason::process_launch_failed);
        std::vector<std::string> argument_storage{options.executable.native()};
        argument_storage.insert(argument_storage.end(), options.arguments.begin(), options.arguments.end());
        std::vector<char *> arguments, variables;
        for (auto &arg : argument_storage)
            arguments.push_back(arg.data());
        arguments.push_back(nullptr);
        for (auto &variable : environment.value())
            variables.push_back(variable.data());
        variables.push_back(nullptr);
        const int status = posix_spawn(&child, options.executable.c_str(), &actions, &attributes,
                                       arguments.data(), variables.data());
        if (status != 0) {
            child = -1;
            return process_error(ErrorReason::process_launch_failed);
        }
        state.identifier = static_cast<std::uint64_t>(child);
        state.running = true;
        native_live = true;
#endif
        return {};
    }

    Result<void> signal_child(bool force) {
        std::lock_guard lock(mutex);
        if (!native_live || (force ? forced : graceful_requested))
            return {};
#ifdef _WIN32
        const bool success =
            force ? (job_assigned ? TerminateJobObject(job.value, 1) != FALSE
                                  : TerminateProcess(process.value, 1) != FALSE)
                  : GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, static_cast<DWORD>(state.identifier)) != FALSE;
#else
        const bool success = kill(-child, force ? SIGKILL : SIGTERM) == 0 || errno == ESRCH;
#endif
        if (!success)
            return process_error(ErrorReason::process_control_failed);
        (force ? forced : graceful_requested) = true;
        return {};
    }
    void append(CapturedOutput &capture, const char *data, std::size_t count) {
        const auto kept = std::min(count, limit - capture.bytes.size());
        capture.bytes.append(data, kept);
        capture.truncated = capture.truncated || kept != count;
    }
    // One bounded drain slice keeps even a noisy child from starving exit checks.
    void drain(bool standard_error, std::size_t budget = 64 * 1024) {
        std::array<char, 4096> buffer{};
        std::lock_guard lock(mutex);
        auto &pipe = standard_error ? errors : output;
        auto &capture = standard_error ? state.errors : state.output;
        while (budget > 0) {
#ifdef _WIN32
            if (!pipe)
                return;
            DWORD available = 0, count = 0;
            if (!PeekNamedPipe(pipe.value, nullptr, 0, nullptr, &available, nullptr)) {
                if (GetLastError() != ERROR_BROKEN_PIPE)
                    state.failure = process_error(ErrorReason::output_read_failed);
                pipe.reset();
                return;
            }
            if (available == 0)
                return;
            const auto wanted =
                static_cast<DWORD>(std::min({budget, buffer.size(), static_cast<std::size_t>(available)}));
            if (!ReadFile(pipe.value, buffer.data(), wanted, &count, nullptr)) {
                if (GetLastError() != ERROR_BROKEN_PIPE)
                    state.failure = process_error(ErrorReason::output_read_failed);
                pipe.reset();
                return;
            }
#else
            if (pipe.value < 0)
                return;
            const auto count = read(pipe.value, buffer.data(), std::min(budget, buffer.size()));
            if (count < 0) {
                if (errno == EINTR)
                    return;
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    state.failure = process_error(ErrorReason::output_read_failed);
                    pipe.reset();
                }
                return;
            }
#endif
            if (count == 0) {
                pipe.reset();
                return;
            }
            append(capture, buffer.data(), static_cast<std::size_t>(count));
            budget -= static_cast<std::size_t>(count);
        }
    }
    std::optional<ProcessExit> collect_exit() {
        std::lock_guard lock(mutex);
#ifdef _WIN32
        const DWORD waited = WaitForSingleObject(process.value, 0);
        if (waited == WAIT_TIMEOUT)
            return {};
        DWORD code = 0;
        if (waited != WAIT_OBJECT_0 || !GetExitCodeProcess(process.value, &code)) {
            state.failure = process_error(ErrorReason::process_wait_failed);
            if (job_assigned)
                TerminateJobObject(job.value, 1);
            else
                TerminateProcess(process.value, 1);
            WaitForSingleObject(process.value, INFINITE);
        } else if (job_assigned && !TerminateJobObject(job.value, 1)) {
            state.failure = process_error(ErrorReason::process_control_failed);
        }
        native_live = false;
        return ProcessExit{code, 0};
#else
        siginfo_t info{};
        // Keep the leader unreaped until group cleanup so its PID/PGID cannot be reused.
        if (waitid(P_PID, static_cast<id_t>(child), &info, WEXITED | WNOHANG | WNOWAIT) != 0) {
            const int wait_error = errno;
            if (wait_error == EINTR)
                return {};
            state.failure = process_error(ErrorReason::process_wait_failed);
            // ECHILD means a foreign reaper violated the ownership contract. Do
            // not signal an identity that may have been recycled in that case.
            if (wait_error != ECHILD) {
                kill(-child, SIGKILL);
                int status = 0;
                while (waitpid(child, &status, 0) < 0 && errno == EINTR) {
                }
            }
            child = -1;
            native_live = false;
            return ProcessExit{};
        }
        if (info.si_pid == 0)
            return {};
        if (kill(-child, SIGKILL) != 0 && errno != ESRCH)
            state.failure = process_error(ErrorReason::process_control_failed);
        int status = 0;
        pid_t waited;
        do {
            waited = waitpid(child, &status, 0);
        } while (waited < 0 && errno == EINTR);
        child = -1;
        native_live = false;
        if (waited < 0) {
            state.failure = process_error(ErrorReason::process_wait_failed);
            return ProcessExit{};
        }
        return WIFEXITED(status) ? ProcessExit{static_cast<std::uint32_t>(WEXITSTATUS(status)), 0}
                                 : ProcessExit{0, WTERMSIG(status)};
#endif
    }
    bool pipes_open() const {
#ifdef _WIN32
        return static_cast<bool>(output) || static_cast<bool>(errors);
#else
        return output.value >= 0 || errors.value >= 0;
#endif
    }
    void run() {
        for (;;) {
            drain(false);
            drain(true);
            auto exit = collect_exit();
            if (exit) {
                // Direct child is reaped; descendants in its group/job were stopped.
                // Escaped descendants cannot keep the monitor alive through a pipe.
                const auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
                while (pipes_open()) {
                    drain(false, max_capture);
                    drain(true, max_capture);
                    if (!pipes_open() || std::chrono::steady_clock::now() >= until)
                        break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
                {
                    std::lock_guard lock(mutex);
#ifdef _WIN32
                    state.output.truncated = state.output.truncated || static_cast<bool>(output);
                    state.errors.truncated = state.errors.truncated || static_cast<bool>(errors);
#else
                    state.output.truncated = state.output.truncated || output.value >= 0;
                    state.errors.truncated = state.errors.truncated || errors.value >= 0;
#endif
                    output.reset();
                    errors.reset();
                    state.running = false;
                    state.exit = exit;
                }
                changed.notify_all();
                return;
            }
#ifdef _WIN32
            WaitForSingleObject(process.value, 20);
#else
            const pollfd descriptors[] = {{output.value, POLLIN, 0}, {errors.value, POLLIN, 0}};
            auto waiting = std::array{descriptors[0], descriptors[1]};
            // Timed blocking poll, not a spin loop; exit-only observation is <=20 ms.
            if (poll(waiting.data(), waiting.size(), 20) < 0 && errno != EINTR)
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
#endif
        }
    }
    ~Impl() noexcept {
        try {
            (void)signal_child(true);
            if (monitor.joinable()) {
                monitor.join();
            } else if (state.running) {
                // Launch succeeded but thread allocation failed; ownership still unwinds.
#ifdef _WIN32
                WaitForSingleObject(process.value, INFINITE);
#else
                int status = 0;
                while (waitpid(child, &status, 0) < 0 && errno == EINTR) {
                }
                child = -1;
#endif
            }
        } catch (...) {
            std::terminate(); // Never detach a live child/monitor on cleanup failure.
        }
    }
};

Process::Process(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
Process::~Process() noexcept = default;
Result<std::unique_ptr<Process>> Process::launch(const ProcessOptions &options, std::stop_token stop) {
    if (stop.stop_requested())
        return process_error(ErrorReason::cancelled);
    try {
        auto valid = validate(options);
        if (!valid)
            return valid.error();
        auto impl = std::make_unique<Impl>();
        auto spawned = impl->spawn(options);
        if (!spawned)
            return spawned.error();
        if (stop.stop_requested())
            return process_error(ErrorReason::cancelled);
        auto *owned = impl.get();
        impl->monitor = std::jthread([owned] { owned->run(); });
        return std::unique_ptr<Process>(new Process(std::move(impl)));
    } catch (const std::filesystem::filesystem_error &) {
        return Error{ErrorCode::usage, "Invalid native process path.", ErrorReason::invalid_process_options};
    } catch (...) {
        return process_error(ErrorReason::resource_exhausted);
    }
}
Result<ProcessExit> Process::wait_for(std::chrono::milliseconds timeout, std::stop_token stop) {
    if (timeout.count() < 0 || timeout > std::chrono::milliseconds(60000))
        return Error{ErrorCode::usage, "Invalid process wait budget.", ErrorReason::invalid_budget};
    const auto until = std::chrono::steady_clock::now() + timeout;
    std::unique_lock lock(impl_->mutex);
    const bool done = impl_->changed.wait_until(lock, stop, until, [&] { return !impl_->state.running; });
    if (!done)
        return process_error(stop.stop_requested() ? ErrorReason::cancelled : ErrorReason::deadline_exceeded);
    auto result = impl_->state.failure ? Result<ProcessExit>(*impl_->state.failure)
                                       : Result<ProcessExit>(*impl_->state.exit);
    lock.unlock();
    std::lock_guard join_lock(impl_->join_mutex);
    if (impl_->monitor.joinable())
        impl_->monitor.join();
    return result;
}
Result<void> Process::request_stop() {
    return impl_->signal_child(false);
}
Result<void> Process::terminate() {
    return impl_->signal_child(true);
}
ProcessSnapshot Process::snapshot() const {
    std::lock_guard lock(impl_->mutex);
    return impl_->state;
}
} // namespace ghostwire::platform
