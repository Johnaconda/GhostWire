// Opt-in, TEST-ONLY wrapper around unmodified pinned TinyCBOR v7.0.
// No sockets, identities, private state, or product runtime linkage.
#include "cbor.h"
#include "bounded_hex_line.hpp"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {
int nibble(char c) noexcept {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}
bool decode_hex(const std::string &hex, std::vector<std::uint8_t> &out) {
    if (hex.size() > (65540U * 2U) || (hex.size() % 2U)) return false;
    out.clear();
    for (std::size_t pos = 0; pos < hex.size(); pos += 2) {
        const auto a = nibble(hex[pos]);
        const auto b = nibble(hex[pos + 1]);
        if (a < 0 || b < 0) return false;
        out.push_back(static_cast<std::uint8_t>(a * 16 + b));
    }
    return true;
}
bool upstream_accepts(const std::vector<std::uint8_t> &frame) {
    if (frame.size() < 5 || frame.size() > 65540) return false;
    const auto len = (std::uint32_t(frame[0]) << 24U) |
        (std::uint32_t(frame[1]) << 16U) | (std::uint32_t(frame[2]) << 8U) | frame[3];
    if (len == 0 || len > 65536 || frame.size() - 4 != len) return false;
    CborParser parser{};
    CborValue item{};
    if (cbor_parser_init(frame.data() + 4, len, 0, &parser, &item) != CborNoError) return false;
    constexpr std::uint32_t flags = CborValidateShortestIntegrals |
        CborValidateNoIndeterminateLength | CborValidateMapIsSorted |
        CborValidateMapKeysAreUnique | CborValidateNoTags | CborValidateCompleteData;
    return cbor_value_validate(&item, flags) == CborNoError;
}
} // namespace

int main() {
    std::string line;
    std::vector<std::uint8_t> frame;
    for (;;) {
        const auto status = ghostwire::test::interop::read_bounded_hex_line(
            std::cin, line, 65540U * 2U);
        if (status == ghostwire::test::interop::LineStatus::end) break;
        // Fixed verdict only; invalid raw bytes never enter stdout or error logs.
        const auto ok = status == ghostwire::test::interop::LineStatus::line &&
                        decode_hex(line, frame) && upstream_accepts(frame);
        std::cout << (ok ? "OK" : "ERR") << '\n';
    }
    return std::cin.bad() ? 1 : 0;
}
