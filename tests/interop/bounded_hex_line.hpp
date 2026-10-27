#pragma once
// Shared, test-only text-framing boundary for in-process codec probes.
// Bounded allocation even when an untrusted producer never sends a newline.
// Neither probe is an application transport or an authentication endpoint.
#include <cstddef>
#include <istream>
#include <string>

namespace ghostwire::test::interop {
enum class LineStatus { line, oversized, end };

inline LineStatus read_bounded_hex_line(std::istream &input, std::string &line,
                                        std::size_t max_chars) {
    line.clear();
    bool seen = false;
    bool too_long = false;
    for (;;) {
        const auto ch = input.get();
        if (ch == std::char_traits<char>::eof()) {
            if (!seen) return LineStatus::end;
            return too_long ? LineStatus::oversized : LineStatus::line;
        }
        seen = true;
        if (ch == '\n') return too_long ? LineStatus::oversized : LineStatus::line;
        if (too_long || line.size() == max_chars) {
            too_long = true;
        } else {
            line.push_back(static_cast<char>(ch));
        }
    }
}
} // namespace ghostwire::test::interop
