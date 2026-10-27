// Test-only isolated byte-buffer fault harness. No global operator new
// replacement, runtime hooks, allocator interception or production test API.
#include "ghostwire/protocol/wire_codec.hpp"
#include <atomic>
#include <cstdint>
#include <new>
#include <vector>

namespace {
// Only the codec's two byte-buffer allocation sites call this test hook.
std::atomic<int> fail_next{-1};
std::atomic<std::size_t> allocation_attempts{0};
void set_failure(bool enabled) noexcept { fail_next.store(enabled ? 0 : -1); }
[[nodiscard]] std::size_t attempts() noexcept {
    return allocation_attempts.load(std::memory_order_relaxed);
}
} // namespace

// C++ linkage is required: MSVC /EHsc assumes extern "C" calls do not
// throw, but this test-only hook deliberately throws std::bad_alloc.
// Referenced only by wire_codec.cpp compiled with GW_PROTOCOL_FAULT_TESTING;
// never linked into production libraries.
void ghostwire_protocol_allocation_point() noexcept(false) {
    allocation_attempts.fetch_add(1, std::memory_order_relaxed);
    if (fail_next.exchange(-1, std::memory_order_relaxed) == 0)
        throw std::bad_alloc{};
}

int main() {
    using namespace ghostwire::protocol;
    Admission admission;
    Envelope public_record{PublicDeviceSet{std::vector<std::uint8_t>(182, 0x5a)}, std::nullopt};
    const auto expected_bytes = std::vector<std::uint8_t>(182, 0x5a);

    // Work and byte-budget denials must occur before entering either byte
    // allocation site; the error alone would not establish this property.
    Operation budget;
    budget.maximum_work = 1;
    const auto before_work = attempts();
    const auto denied_work = encode_frame(public_record, admission, budget);
    if (denied_work || denied_work.error().code != ErrorCode::resource_limit ||
        attempts() != before_work) return 7;
    budget.maximum_work = maximum_frame_bytes + 512;
    budget.maximum_bytes = 1;
    const auto before_bytes = attempts();
    const auto denied_bytes = encode_frame(public_record, admission, budget);
    if (denied_bytes || denied_bytes.error().code != ErrorCode::frame_limit ||
        attempts() != before_bytes) return 8;

    // Inject std::bad_alloc at the real encode reserve boundary. The lease
    // must unwind, failure must be typed, and the input must be untouched.
    const auto before_encode_fault = attempts();
    set_failure(true);
    const auto failed_encode = encode_frame(public_record, admission);
    set_failure(false);
    if (failed_encode || failed_encode.error().code != ErrorCode::resource_limit ||
        attempts() != before_encode_fault + 1) return 1;
    if (std::get<PublicDeviceSet>(public_record.body).frame != expected_bytes) return 6;
    const auto wire = encode_frame(public_record, admission);
    if (!wire || attempts() != before_encode_fault + 2) return 2;

    // Decoder's denied work budget must reject before copying the public
    // DeviceSet bytes or entering the allocation site.
    Operation low_decode_budget;
    low_decode_budget.maximum_work = 1;
    const auto before_decode_limit = attempts();
    const auto denied_decode = decode_frame(wire.value(), admission, low_decode_budget);
    if (denied_decode || denied_decode.error().code != ErrorCode::resource_limit ||
        attempts() != before_decode_limit) return 9;

    const auto before_decode_fault = attempts();
    set_failure(true);
    const auto failed_decode = decode_frame(wire.value(), admission);
    set_failure(false);
    if (failed_decode || failed_decode.error().code != ErrorCode::resource_limit ||
        attempts() != before_decode_fault + 1) return 3;
    const auto decoded = decode_frame(wire.value(), admission);
    if (!decoded || decoded.value() != public_record ||
        attempts() != before_decode_fault + 2) return 4;

    // A failed encode/decode must release the admission slot; the full
    // roundtrip is still viable and no caller-owned input was modified.
    if (!admission.try_acquire()) return 5;
    admission.release();
    if (std::get<PublicDeviceSet>(public_record.body).frame != expected_bytes) return 10;
    const auto reencoded = encode_frame(decoded.value(), admission);
    if (!reencoded || reencoded.value() != wire.value()) return 11;
    return 0;
}
