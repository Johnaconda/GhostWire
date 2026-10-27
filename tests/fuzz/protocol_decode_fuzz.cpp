// Optional external libFuzzer entrypoint, deliberately not wired into standard MSVC builds.
#include "ghostwire/protocol/wire_codec.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size) {
    ghostwire::protocol::Admission admission;
    const auto result = ghostwire::protocol::decode_frame(std::span(data, size), admission);
    (void)result;
    return 0;
}
