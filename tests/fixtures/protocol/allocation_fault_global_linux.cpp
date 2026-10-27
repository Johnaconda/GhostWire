// Linux-only stronger allocation-observation regression. Preserves the original
// HF07 process-local global allocator override on the platform where it is
// known to terminate; the portable Windows fixture uses scoped test hooks.
// No runtime fault hooks.
#include "ghostwire/protocol/wire_codec.hpp"
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <span>
#include <vector>

namespace {
std::atomic<int> fail_at{-1};
std::atomic<std::size_t> allocation_calls{0};
void *alloc(std::size_t n) {
    allocation_calls.fetch_add(1, std::memory_order_relaxed);
    const auto count = fail_at.load(std::memory_order_relaxed);
    if (count >= 0 && fail_at.fetch_sub(1, std::memory_order_relaxed) == 0) {
        fail_at.store(-1, std::memory_order_relaxed);
        throw std::bad_alloc{};
    }
    if (void *p = std::malloc(n ? n : 1)) return p;
    throw std::bad_alloc{};
}
void set_failure(int n) noexcept { fail_at.store(n, std::memory_order_relaxed); }
} // namespace

void *operator new(std::size_t n) { return alloc(n); }
void *operator new[](std::size_t n) { return alloc(n); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }

int main() {
    using namespace ghostwire::protocol;
    Admission admission;
    Envelope public_record{PublicDeviceSet{std::vector<std::uint8_t>(182, 0x5a)}, std::nullopt};
    // Both resource ceilings must deny the operation *before* allocating.
    // Record allocations directly; an error-code-only test cannot distinguish
    // preflight admission from an allocation that was later rolled back.
    Operation budget;
    budget.maximum_work = 1;
    const auto before_work = allocation_calls.load(std::memory_order_relaxed);
    const auto denied_work = encode_frame(public_record, admission, budget);
    if (denied_work || denied_work.error().code != ErrorCode::resource_limit ||
        allocation_calls.load(std::memory_order_relaxed) != before_work) return 7;
    budget.maximum_work = maximum_frame_bytes + 512;
    budget.maximum_bytes = 1;
    const auto before_bytes = allocation_calls.load(std::memory_order_relaxed);
    const auto denied_bytes = encode_frame(public_record, admission, budget);
    if (denied_bytes || denied_bytes.error().code != ErrorCode::frame_limit ||
        allocation_calls.load(std::memory_order_relaxed) != before_bytes) return 8;
    set_failure(0);
    const auto failed_encode = encode_frame(public_record, admission);
    set_failure(-1);
    if (failed_encode || failed_encode.error().code != ErrorCode::resource_limit) return 1;
    const auto wire = encode_frame(public_record, admission);
    if (!wire) return 2;
    // Decoder must reject a frame on a denied work budget without copying
    // the DeviceSet bytes or allocating any result storage.
    Operation low_decode_budget;
    low_decode_budget.maximum_work = 1;
    const auto before_decode_limit = allocation_calls.load(std::memory_order_relaxed);
    const auto denied_decode = decode_frame(wire.value(), admission, low_decode_budget);
    if (denied_decode || denied_decode.error().code != ErrorCode::resource_limit ||
        allocation_calls.load(std::memory_order_relaxed) != before_decode_limit) return 9;
    set_failure(0);
    const auto failed_decode = decode_frame(wire.value(), admission);
    set_failure(-1);
    if (failed_decode || failed_decode.error().code != ErrorCode::resource_limit) return 3;
    const auto decoded = decode_frame(wire.value(), admission);
    if (!decoded || decoded.value() != public_record) return 4;
    // A fresh call after an allocation failure must not observe a leaked lease.
    if (!admission.try_acquire()) return 5;
    admission.release();
    // Failures do not change the caller's public input bytes or identity state.
    if (std::get<PublicDeviceSet>(public_record.body).frame != std::vector<std::uint8_t>(182, 0x5a))
        return 6;
    return 0;
}
