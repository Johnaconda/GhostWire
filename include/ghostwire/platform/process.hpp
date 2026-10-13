#pragma once
#include "ghostwire/core/result.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>
#include <vector>

namespace ghostwire::platform {
enum class ProcessEnvironment { inherit, replace };
enum class ProcessOutput { discard, capture };
struct EnvironmentVariable {
    std::string name, value;
};
struct ProcessOptions {
    // Absolute native executable/cwd; no PATH search, shell or script expansion.
    std::filesystem::path executable, working_directory;
    std::vector<std::string> arguments;
    ProcessEnvironment environment = ProcessEnvironment::replace;
    std::vector<EnvironmentVariable> variables;
    ProcessOutput output = ProcessOutput::discard;
    std::size_t capture_bytes_per_stream = 64 * 1024;
};
struct ProcessExit {
    // On POSIX, signaled exits have signal != 0 and code == 0.
    std::uint32_t code = 0;
    int signal = 0;
    bool operator==(const ProcessExit &) const = default;
};
struct CapturedOutput {
    // Raw, potentially sensitive bytes. Never automatically decoded or logged.
    std::string bytes;
    bool truncated = false;
};
struct ProcessSnapshot {
    std::uint64_t identifier = 0; // Diagnostic only; cannot grant control authority.
    bool running = false;
    std::optional<ProcessExit> exit;
    CapturedOutput output, errors;
    std::optional<core::Error> failure;
};
// Owns one child, its process group/job, pipes and one joined monitor thread.
// Operations are synchronized; destruction must not race callers. Never adopts
// foreign PIDs. Trusted local children only: this is not an execution sandbox.
class Process {
  public:
    static core::Result<std::unique_ptr<Process>> launch(const ProcessOptions &options,
                                                         std::stop_token stop = {});
    ~Process() noexcept;
    Process(const Process &) = delete;
    Process &operator=(const Process &) = delete;
    // 0..60,000 ms. Timeout/cancellation stops the WAIT, retaining child ownership.
    core::Result<ProcessExit> wait_for(std::chrono::milliseconds timeout, std::stop_token stop = {});
    // SIGTERM to owned group / CTRL_BREAK to owned Windows console group.
    // Windows console-less children can refuse this; terminate remains explicit.
    core::Result<void> request_stop();
    core::Result<void> terminate(); // Force only the owned group/job; repeat-safe.
    [[nodiscard]] ProcessSnapshot snapshot() const;

  private:
    struct Impl;
    explicit Process(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> impl_;
};
} // namespace ghostwire::platform
