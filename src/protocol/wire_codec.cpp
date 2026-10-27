#include "ghostwire/protocol/wire_codec.hpp"
#include <algorithm>
#include <limits>
#include <new>
#include <utility>

// This hook exists only in the separately compiled allocation-fault test
// executable. Production ghostwire_protocol contains no fault-injection hook.
#if defined(GW_PROTOCOL_FAULT_TESTING)
void ghostwire_protocol_allocation_point() noexcept(false);
#endif

namespace ghostwire::protocol {
namespace {
using Bytes = std::vector<std::uint8_t>;
// The restricted wire codec has exactly two owned byte-buffer allocation
// sites. In the fault fixture, inject std::bad_alloc immediately before the
// corresponding vector operation. This tests exception cleanup without
// replacing global operator new (which also intercepts MSVC CRT activity).
inline void allocation_point() {
#if defined(GW_PROTOCOL_FAULT_TESTING)
    ::ghostwire_protocol_allocation_point();
#endif
}
class Lease {
  public:
    explicit Lease(Admission &a) noexcept : owner_(a), acquired_(a.try_acquire()) {}
    Lease(const Lease &) = delete;
    Lease &operator=(const Lease &) = delete;
    ~Lease() { if (acquired_) owner_.release(); }
    [[nodiscard]] bool acquired() const noexcept { return acquired_; }
  private:
    Admission &owner_;
    bool acquired_;
};
constexpr Error fail(ErrorCode code) noexcept { return Error{code}; }

// Restricted profile-specific CBOR tokens; deliberately NOT a general CBOR parser.
// All length arithmetic is bounded before span indexing or allocation.
class Reader {
  public:
    Reader(std::span<const std::uint8_t> input, const Operation &options) noexcept
        : input_(input), options_(options) {}
    [[nodiscard]] ErrorCode state() const noexcept { return error_.value_or(ErrorCode::malformed_frame); }
    [[nodiscard]] bool good() const noexcept { return !error_.has_value(); }
    [[nodiscard]] bool done() const noexcept { return offset_ == input_.size(); }
    [[nodiscard]] bool check(std::size_t charge = 1) noexcept {
        if (!good()) return false;
        if (options_.stop.stop_requested()) { error_ = ErrorCode::cancelled; return false; }
        if (std::chrono::steady_clock::now() >= options_.deadline) {
            error_ = ErrorCode::deadline_exceeded; return false;
        }
        const auto max_work = std::min(options_.maximum_work, maximum_frame_bytes + 512);
        if (charge > max_work - std::min(work_, max_work)) {
            error_ = ErrorCode::resource_limit; return false;
        }
        work_ += charge;
        if (++items_ > 512) { error_ = ErrorCode::resource_limit; return false; }
        return true;
    }
    [[nodiscard]] bool token(std::uint8_t major, std::uint64_t &value, std::uint64_t maximum) noexcept {
        if (!check()) return false;
        if (offset_ == input_.size()) return bad(ErrorCode::malformed_frame);
        const auto initial = input_[offset_++];
        const auto type = static_cast<std::uint8_t>(initial >> 5U);
        const auto info = static_cast<std::uint8_t>(initial & 31U);
        if (info >= 28) return bad(info == 31 ? ErrorCode::noncanonical_encoding : ErrorCode::malformed_frame);
        if (type != major) return bad(ErrorCode::malformed_frame);
        std::size_t width = 0;
        if (info == 24) width = 1;
        if (info == 25) width = 2;
        if (info == 26) width = 4;
        if (info == 27) width = 8;
        if (width > input_.size() - offset_) return bad(ErrorCode::malformed_frame);
        value = info < 24 ? info : 0;
        for (std::size_t j = 0; j < width; ++j) value = (value << 8U) | input_[offset_++];
        if ((info == 24 && value < 24) || (info == 25 && value <= 255) ||
            (info == 26 && value <= 65535) || (info == 27 && value <= 0xffffffffULL))
            return bad(ErrorCode::noncanonical_encoding);
        if (value > maximum) return bad(ErrorCode::resource_limit);
        return true;
    }
    [[nodiscard]] bool integer(std::uint64_t &v) noexcept {
        return token(0, v, std::numeric_limits<std::uint64_t>::max());
    }
    [[nodiscard]] bool collection(std::uint8_t type, std::uint64_t &length, std::uint64_t max_length) noexcept {
        if (!token(type, length, 512)) return false;
        if (length > max_length) return bad(ErrorCode::resource_limit);
        return true;
    }
    [[nodiscard]] bool key(std::uint64_t &previous, bool &seen, std::uint64_t &value) noexcept {
        if (offset_ == input_.size()) return bad(ErrorCode::malformed_frame);
        const auto initial = input_[offset_];
        if (!integer(value)) return false;
        if (initial >= 24 && value <= 23) return bad(ErrorCode::noncanonical_encoding);
        if (seen && value <= previous) return bad(ErrorCode::noncanonical_encoding);
        previous = value; seen = true;
        return value <= 23 ? true : bad(ErrorCode::malformed_frame);
    }
    [[nodiscard]] bool bytes(std::span<const std::uint8_t> &value, std::size_t max_length) noexcept {
        std::uint64_t length = 0;
        if (!token(2, length, 49152)) return false;
        if (length > max_length) return bad(ErrorCode::resource_limit);
        if (length > input_.size() - offset_) return bad(ErrorCode::malformed_frame);
        if (!check(static_cast<std::size_t>(length))) return false;
        value = input_.subspan(offset_, static_cast<std::size_t>(length));
        offset_ += static_cast<std::size_t>(length);
        return true;
    }
    [[nodiscard]] bool bad(ErrorCode error) noexcept { error_ = error; return false; }
  private:
    std::span<const std::uint8_t> input_;
    const Operation &options_;
    std::size_t offset_ = 0;
    std::size_t items_ = 0;
    std::size_t work_ = 0;
    std::optional<ErrorCode> error_;
};

// Fixed grammar has maximum container depth four: envelope, body, versions, pair.
// No recursion or unknown-field skipping. Key-order checks occur at each map.
[[nodiscard]] bool version_pair(Reader &r, Version &pair) noexcept {
    std::uint64_t count = 0;
    if (!r.collection(4, count, 64) || count != 2) return false;
    return r.integer(pair.major) && r.integer(pair.minor);
}
[[nodiscard]] bool capabilities(Reader &r, std::size_t &count) noexcept {
    std::uint64_t n = 0;
    if (!r.collection(4, n, 32)) return false;
    count = static_cast<std::size_t>(n);
    std::uint64_t previous = 0;
    for (std::size_t i = 0; i < count; ++i) {
        std::uint64_t current = 0;
        if (!r.integer(current)) return false;
        if (i > 0 && current <= previous) return r.bad(ErrorCode::noncanonical_encoding);
        previous = current;
    }
    return count == 0 ? true : r.bad(ErrorCode::unsupported_capability);
}
[[nodiscard]] bool body_offer(Reader &r, VersionOffer &o) noexcept {
    std::uint64_t n = 0, prev = 0, key = 0;
    bool seen = false;
    if (!r.collection(5, n, 16) || (n != 2 && n != 3)) return false;
    const auto map_count = n;
    if (!r.key(prev, seen, key)) return false;
    if (key != 0) return r.bad(key <= 1 ? ErrorCode::noncanonical_encoding : ErrorCode::malformed_frame);
    if (!r.collection(4, n, 8) || n == 0) return false;
    o.count = static_cast<std::size_t>(n);
    for (std::size_t i = 0; i < o.count; ++i) {
        if (!version_pair(r, o.versions[i])) return false;
        if (i > 0 && !(o.versions[i - 1].major < o.versions[i].major ||
                        (o.versions[i - 1].major == o.versions[i].major &&
                         o.versions[i - 1].minor < o.versions[i].minor)))
            return r.bad(ErrorCode::noncanonical_encoding);
    }
    if (!r.key(prev, seen, key) || key != 1) return false;
    if (!capabilities(r, o.capability_count)) return false;
    if (map_count == 3) {
        if (!r.key(prev, seen, key)) return false;
        return r.bad(ErrorCode::malformed_frame);
    }
    for (std::size_t i = 0; i < o.count; ++i)
        if (o.versions[i] != Version{}) return r.bad(ErrorCode::unsupported_version);
    return o.count == 1;
}
[[nodiscard]] bool body_select(Reader &r, VersionSelect &s) noexcept {
    std::uint64_t n = 0, prev = 0, key = 0;
    bool seen = false;
    if (!r.collection(5, n, 16) || (n != 2 && n != 3)) return false;
    const auto map_count = n;
    if (!r.key(prev, seen, key)) return false;
    if (key != 0) return r.bad(key <= 1 ? ErrorCode::noncanonical_encoding : ErrorCode::malformed_frame);
    if (!version_pair(r, s.selected)) return false;
    if (!r.key(prev, seen, key) || key != 1 || !capabilities(r, s.capability_count)) return false;
    if (map_count == 3) {
        if (!r.key(prev, seen, key)) return false;
        return r.bad(ErrorCode::malformed_frame);
    }
    return s.selected == Version{} ? true : r.bad(ErrorCode::unsupported_version);
}
[[nodiscard]] bool body_device(Reader &r, PublicDeviceSet &d) {
    std::uint64_t n = 0, prev = 0, key = 0;
    bool seen = false;
    if (!r.collection(5, n, 16) || n != 1 || !r.key(prev, seen, key) || key != 0) return false;
    std::span<const std::uint8_t> b;
    if (!r.bytes(b, maximum_device_set_bytes)) return false;
    if (b.size() < minimum_device_set_bytes) return r.bad(ErrorCode::malformed_frame);
    // Full DeviceSet semantic validation and signatures belong to A04.
    allocation_point();
    d.frame.assign(b.begin(), b.end());
    return true;
}
[[nodiscard]] bool routing(Reader &r, RoutingHint &hint) noexcept {
    std::uint64_t n = 0, previous = 0, key = 0;
    bool seen = false;
    if (!r.collection(5, n, 16) || n < 1 || n > 2) return false;
    if (!r.key(previous, seen, key) || key != 0) return false;
    std::span<const std::uint8_t> token;
    if (!r.bytes(token, 16) || token.size() != 16) return false;
    std::copy(token.begin(), token.end(), hint.token.begin());
    if (n == 2) {
        std::uint64_t hop = 0;
        if (!r.key(previous, seen, key) || key != 1 || !r.integer(hop)) return false;
        if (hop > 16) return r.bad(ErrorCode::malformed_frame);
        hint.hop_limit = static_cast<std::uint8_t>(hop);
    }
    return true;
}

// Canonical unsigned CBOR emitters: always shortest-width, integer big endian.
void header(Bytes &out, std::uint8_t major, std::uint64_t value) {
    const std::uint8_t prefix = static_cast<std::uint8_t>(major << 5U);
    if (value < 24) { out.push_back(static_cast<std::uint8_t>(prefix | value)); return; }
    std::size_t width = 1;
    std::uint8_t info = 24;
    if (value > 0xff) { width = 2; info = 25; }
    if (value > 0xffff) { width = 4; info = 26; }
    if (value > 0xffffffffULL) { width = 8; info = 27; }
    out.push_back(static_cast<std::uint8_t>(prefix | info));
    for (std::size_t j = width; j > 0; --j)
        out.push_back(static_cast<std::uint8_t>((value >> ((j - 1) * 8U)) & 0xffU));
}
void number(Bytes &out, std::uint64_t n) { header(out, 0, n); }
void pair(Bytes &out, Version v) { header(out, 4, 2); number(out, v.major); number(out, v.minor); }
} // namespace

bool selected_was_offered(const VersionOffer &offer, const VersionSelect &select) noexcept {
    // The only admissible v1.0 offer is the single registered version and
    // no capabilities.  A caller-created, impossible offer must not pass the
    // advisory selection check even if it happens to contain [1,0].
    return offer.count == 1 && offer.versions[0] == Version{} &&
           offer.capability_count == 0 && select.capability_count == 0 &&
           select.selected == Version{};
}

Result<Envelope> decode_frame(std::span<const std::uint8_t> frame, Admission &admission,
                              const Operation &options) {
    Lease lease(admission);
    if (!lease.acquired()) return fail(ErrorCode::operation_busy);
    if (options.stop.stop_requested()) return fail(ErrorCode::cancelled);
    if (std::chrono::steady_clock::now() >= options.deadline) return fail(ErrorCode::deadline_exceeded);
    if (frame.size() < 4) return fail(ErrorCode::incomplete_frame);
    const auto declared = (static_cast<std::uint32_t>(frame[0]) << 24U) |
                          (static_cast<std::uint32_t>(frame[1]) << 16U) |
                          (static_cast<std::uint32_t>(frame[2]) << 8U) | frame[3];
    if (declared == 0 || declared > maximum_frame_bytes || declared > options.maximum_bytes)
        return fail(ErrorCode::frame_limit);
    if (static_cast<std::size_t>(declared) > frame.size() - 4) return fail(ErrorCode::incomplete_frame);
    if (static_cast<std::size_t>(declared) != frame.size() - 4) return fail(ErrorCode::malformed_frame);
    Reader r(frame.subspan(4), options);
    try {
        std::uint64_t n = 0, previous = 0, key = 0, major = 0, minor = 0, kind = 0;
        bool seen = false;
        if (!r.collection(5, n, 16) || (n != 4 && n != 5)) return fail(r.state());
        if (!r.key(previous, seen, key)) return fail(r.state());
        if (key != 0) return fail(key <= 4 ? ErrorCode::noncanonical_encoding : ErrorCode::malformed_frame);
        if (!r.integer(major)) return fail(r.state());
        if (!r.key(previous, seen, key) || key != 1 || !r.integer(minor)) return fail(r.state());
        if (major != 1 || minor != 0) return fail(ErrorCode::unsupported_version);
        if (!r.key(previous, seen, key) || key != 2 || !r.integer(kind)) return fail(r.state());
        if (kind != 1 && kind != 2 && kind != 10) return fail(ErrorCode::unsupported_kind);
        if (!r.key(previous, seen, key) || key != 3) return fail(r.state());
        Envelope result;
        if (kind == 1) { VersionOffer offer; if (!body_offer(r, offer)) return fail(r.state()); result.body = offer; }
        if (kind == 2) { VersionSelect select; if (!body_select(r, select)) return fail(r.state()); result.body = select; }
        if (kind == 10) { PublicDeviceSet d; if (!body_device(r, d)) return fail(r.state()); result.body = std::move(d); }
        if (n == 5) {
            if (!r.key(previous, seen, key) || key != 4) return fail(r.state());
            RoutingHint route;
            if (!routing(r, route)) return fail(r.state());
            result.routing = route;
        }
        if (!r.done()) return fail(ErrorCode::malformed_frame);
        if (options.stop.stop_requested()) return fail(ErrorCode::cancelled);
        if (std::chrono::steady_clock::now() >= options.deadline) return fail(ErrorCode::deadline_exceeded);
        return result;
    } catch (const std::bad_alloc &) {
        return fail(ErrorCode::resource_limit);
    }
}

Result<Bytes> encode_frame(const Envelope &message, Admission &admission, const Operation &options) {
    Lease lease(admission);
    if (!lease.acquired()) return fail(ErrorCode::operation_busy);
    if (options.stop.stop_requested()) return fail(ErrorCode::cancelled);
    if (std::chrono::steady_clock::now() >= options.deadline) return fail(ErrorCode::deadline_exceeded);
    const auto *o = std::get_if<VersionOffer>(&message.body);
    const auto *s = std::get_if<VersionSelect>(&message.body);
    const auto *d = std::get_if<PublicDeviceSet>(&message.body);
    if (o) {
        if (o->count != 1 || o->versions[0] != Version{}) return fail(ErrorCode::unsupported_version);
        if (o->capability_count != 0) return fail(ErrorCode::unsupported_capability);
    }
    if (s) {
        if (s->selected != Version{}) return fail(ErrorCode::unsupported_version);
        if (s->capability_count != 0) return fail(ErrorCode::unsupported_capability);
    }
    if (d && (d->frame.size() < minimum_device_set_bytes || d->frame.size() > maximum_device_set_bytes))
        return fail(ErrorCode::malformed_frame);
    if (message.routing && message.routing->hop_limit && *message.routing->hop_limit > 16)
        return fail(ErrorCode::malformed_frame);
    // The wire grammar is closed and its serialized size is fully known
    // before allocation. Respect caller byte/work budgets before reserve(),
    // including a valid but resource-exhausted public record.
    const std::size_t body_bytes = o ? 16U : s ? 15U :
        d->frame.size() + (d->frame.size() <= 255U ? 12U : 13U);
    const std::size_t route_bytes = !message.routing ? 0U :
        (message.routing->hop_limit.has_value() ? 22U : 20U);
    const std::size_t planned_body = body_bytes + route_bytes;
    const std::size_t planned_frame = planned_body + 4U;
    if (planned_body > maximum_frame_bytes || planned_body > options.maximum_bytes)
        return fail(ErrorCode::frame_limit);
    if (planned_frame > std::min(options.maximum_work, maximum_frame_bytes + 512))
        return fail(ErrorCode::resource_limit);
    try {
        Bytes output;
        allocation_point();
        output.reserve(planned_frame);
        output.insert(output.end(), 4, 0); // Staged prefix, filled only after complete body.
        header(output, 5, message.routing ? 5 : 4);
        number(output, 0); number(output, 1);
        number(output, 1); number(output, 0);
        number(output, 2); number(output, o ? 1 : s ? 2 : 10);
        number(output, 3);
        if (o || s) {
            header(output, 5, 2);
            number(output, 0);
            if (o) { header(output, 4, 1); pair(output, o->versions[0]); }
            else pair(output, s->selected);
            number(output, 1); header(output, 4, 0);
        } else {
            header(output, 5, 1); number(output, 0);
            header(output, 2, d->frame.size());
            output.insert(output.end(), d->frame.begin(), d->frame.end());
        }
        if (message.routing) {
            number(output, 4);
            header(output, 5, message.routing->hop_limit ? 2 : 1);
            number(output, 0); header(output, 2, 16);
            output.insert(output.end(), message.routing->token.begin(), message.routing->token.end());
            if (message.routing->hop_limit) { number(output, 1); number(output, *message.routing->hop_limit); }
        }
        const std::size_t length = output.size() - 4;
        if (output.size() != planned_frame || length != planned_body)
            return fail(ErrorCode::malformed_frame); // internal invariant; fail closed
        output[0] = static_cast<std::uint8_t>((length >> 24U) & 255U);
        output[1] = static_cast<std::uint8_t>((length >> 16U) & 255U);
        output[2] = static_cast<std::uint8_t>((length >> 8U) & 255U);
        output[3] = static_cast<std::uint8_t>(length & 255U);
        if (options.stop.stop_requested()) return fail(ErrorCode::cancelled);
        if (std::chrono::steady_clock::now() >= options.deadline) return fail(ErrorCode::deadline_exceeded);
        return output;
    } catch (const std::bad_alloc &) {
        return fail(ErrorCode::resource_limit);
    }
}
} // namespace ghostwire::protocol
