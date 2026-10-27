// Test-only line protocol for independently exercising the actual C++ decoder.
// NEVER connect this tool to sockets or give its output a trust/auth role.
#include "ghostwire/protocol/wire_codec.hpp"
#include "../../interop/bounded_hex_line.hpp"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {
int nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
char hex_digit(std::uint8_t n) { return "0123456789abcdef"[n]; }
}  // namespace

int main() {
    namespace protocol = ghostwire::protocol;
    protocol::Admission admission;
    std::string line;
    for (;;) {
        // Reject a too-long line without ever allocating its full content.
        const auto line_status = ghostwire::test::interop::read_bounded_hex_line(
            std::cin, line, 2 * (protocol::maximum_frame_bytes + 4));
        if (line_status == ghostwire::test::interop::LineStatus::end) break;
        if (line_status == ghostwire::test::interop::LineStatus::oversized || (line.size() & 1U)) {
            std::cout << "INVALID_HEX\n";
            continue;
        }
        std::vector<std::uint8_t> frame;
        frame.reserve(line.size() / 2);
        bool valid = true;
        for (std::size_t i = 0; i < line.size(); i += 2) {
            const int hi = nibble(line[i]);
            const int lo = nibble(line[i + 1]);
            if (hi < 0 || lo < 0) { valid = false; break; }
            frame.push_back(static_cast<std::uint8_t>((hi << 4) | lo));
        }
        if (!valid) { std::cout << "INVALID_HEX\n"; continue; }
        auto decoded = protocol::decode_frame(frame, admission);
        if (!decoded) {
            std::cout << "ERR " << static_cast<unsigned>(decoded.error().code) << '\n';
            continue;
        }
        auto canonical = protocol::encode_frame(decoded.value(), admission);
        if (!canonical) { std::cout << "ENCODER_FAILURE\n"; continue; }
        std::cout << "OK ";
        for (auto b : canonical.value()) {
            std::cout << hex_digit(b >> 4) << hex_digit(b & 15);
        }
        std::cout << '\n';
    }
    return std::cin.bad() ? 1 : 0;
}
