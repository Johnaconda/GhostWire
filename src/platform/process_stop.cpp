#include "ghostwire/platform/process_stop.hpp"
#include <atomic>
#include <cstdint>
#include <limits>
#include <mutex>
#include <thread>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <csignal>
#endif

namespace ghostwire::platform {
using core::Error;
using core::ErrorCode;
using core::ErrorReason;
using core::Result;
namespace {
// The OS registration is necessarily process-wide. This is a private callback
// latch, not a service registry: one explicit owner, no pointer to an object.
// Generation tags prevent callbacks holding old latch state from changing a
// later registration. In-flight OS events are not an ordered event queue.
std::mutex registration_mutex;
bool registered = false;
std::uint64_t generation = 0;
constexpr std::uint64_t active_bit = 4, event_mask = 3;
std::atomic<std::uint64_t> latch{0};
static_assert(decltype(latch)::is_always_lock_free, "Native stop callbacks need lock-free atomics");
bool record(ProcessStopEvent event) noexcept {
    auto value = latch.load(std::memory_order_relaxed);
    if (!(value & active_bit))
        return false;
    if (!(value & event_mask)) {
        const auto next = value | static_cast<std::uint64_t>(event);
        // A single attempt: another signal wins, or teardown replaced the epoch.
        (void)latch.compare_exchange_strong(value, next, std::memory_order_relaxed);
    }
    return true;
}
#ifdef _WIN32
BOOL WINAPI control_handler(DWORD event) {
    if (event == CTRL_C_EVENT)
        return record(ProcessStopEvent::interrupt) ? TRUE : FALSE;
    if (event == CTRL_BREAK_EVENT)
        return record(ProcessStopEvent::terminate) ? TRUE : FALSE;
    // Close/logoff/shutdown have native hard deadlines; do not promise an async
    // cleanup handshake for them by returning TRUE and exiting the callback.
    return FALSE;
}
#else
extern "C" void stop_handler(int signal) {
    (void)record(signal == SIGINT ? ProcessStopEvent::interrupt : ProcessStopEvent::terminate);
}
#endif
Error bridge_error(ErrorReason reason) {
    return {ErrorCode::internal, "Process stop bridge operation failed.", reason};
}
} // namespace
struct ProcessStopBridge::Impl {
    std::stop_source stop;
    std::thread::id owner = std::this_thread::get_id();
    std::uint64_t epoch = 0;
    bool open = false;
    ProcessStopEvent observed = ProcessStopEvent::none;
#ifndef _WIN32
    struct sigaction previous_int{}, previous_term{};
#endif
};
ProcessStopBridge::ProcessStopBridge(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
Result<std::unique_ptr<ProcessStopBridge>> ProcessStopBridge::install() {
    auto bridge = std::unique_ptr<ProcessStopBridge>(new ProcessStopBridge(std::make_unique<Impl>()));
    std::lock_guard lock(registration_mutex);
    if (registered)
        return bridge_error(ErrorReason::invalid_state);
    if (generation == (std::numeric_limits<std::uint64_t>::max() >> 3))
        return bridge_error(ErrorReason::resource_exhausted);
    bridge->impl_->epoch = ++generation << 3;
    latch.store(bridge->impl_->epoch | active_bit, std::memory_order_relaxed);
#ifdef _WIN32
    if (!SetConsoleCtrlHandler(control_handler, TRUE)) {
        latch.store(0, std::memory_order_relaxed);
        return bridge_error(ErrorReason::native_failure);
    }
#else
    struct sigaction action{};
    action.sa_handler = stop_handler;
    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGINT);
    sigaddset(&action.sa_mask, SIGTERM);
    if (sigaction(SIGINT, &action, &bridge->impl_->previous_int) != 0) {
        latch.store(0, std::memory_order_relaxed);
        return bridge_error(ErrorReason::native_failure);
    }
    if (sigaction(SIGTERM, &action, &bridge->impl_->previous_term) != 0) {
        // A failed install must not leave an unowned handler behind. Match the
        // destructor contract if the OS also refuses restoration of SIGINT.
        if (sigaction(SIGINT, &bridge->impl_->previous_int, nullptr) != 0)
            std::terminate();
        latch.store(0, std::memory_order_relaxed);
        return bridge_error(ErrorReason::native_failure);
    }
#endif
    registered = true;
    bridge->impl_->open = true;
    return bridge;
}
Result<ProcessStopEvent> ProcessStopBridge::dispatch() {
    if (impl_->owner != std::this_thread::get_id() || !impl_->open)
        return bridge_error(ErrorReason::invalid_state);
    const auto value = latch.load(std::memory_order_relaxed);
    if ((value & ~std::uint64_t{7}) == impl_->epoch && (value & event_mask)) {
        impl_->observed = static_cast<ProcessStopEvent>(value & event_mask);
        impl_->stop.request_stop(); // Executes user callbacks only on the owner thread.
    }
    return impl_->observed;
}
Result<void> ProcessStopBridge::close() {
    if (!impl_->open)
        return {};
    if (impl_->owner != std::this_thread::get_id())
        return bridge_error(ErrorReason::invalid_state);
    std::lock_guard lock(registration_mutex);
#ifdef _WIN32
    if (!SetConsoleCtrlHandler(control_handler, FALSE))
        return bridge_error(ErrorReason::native_failure);
#else
    if (sigaction(SIGINT, &impl_->previous_int, nullptr) != 0 ||
        sigaction(SIGTERM, &impl_->previous_term, nullptr) != 0)
        return bridge_error(ErrorReason::native_failure);
#endif
    latch.store(0, std::memory_order_relaxed);
    impl_->open = false;
    registered = false;
    return {};
}
std::stop_token ProcessStopBridge::stop_token() const noexcept {
    return impl_->stop.get_token();
}
ProcessStopBridge::~ProcessStopBridge() noexcept {
    if (!close())
        std::terminate(); // Cannot safely silently abandon an owned native registration.
}
} // namespace ghostwire::platform
