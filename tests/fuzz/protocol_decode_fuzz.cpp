// Optional external libFuzzer entrypoint, deliberately not wired into standard MSVC builds.
#include "ghostwire/protocol/wire_codec.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <span>

namespace {
// Owner: coordinator, GW-A05-FUZZ-INVARIANTS. Keep checks active under NDEBUG;
// a failed property must be a reproducible fuzz finding, never a silent success.
void require(bool condition) {
    if (!condition) std::abort();
}

void require_released(ghostwire::protocol::Admission &admission) {
    require(admission.try_acquire());
    admission.release();
}
} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size) {
    const std::span frame(data, size);
    ghostwire::protocol::Admission admission;
    const auto decoded = ghostwire::protocol::decode_frame(frame, admission);
    require_released(admission); // Rejected inputs must also release their lease.

    if (decoded) {
        const auto encoded = ghostwire::protocol::encode_frame(decoded.value(), admission);
        require_released(admission);
        require(encoded.has_value());
        // The closed canonical schema has exactly one encoding for this value.
        require(std::equal(encoded.value().begin(), encoded.value().end(), frame.begin(), frame.end()));
        const auto repeated = ghostwire::protocol::decode_frame(encoded.value(), admission);
        require_released(admission);
        require(repeated.has_value());
        require(repeated.value() == decoded.value());
    }
    return 0;
}
