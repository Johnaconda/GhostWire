#include "ghostwire/permissions/local_policy.hpp"
#include <limits>

namespace ghostwire::permissions {
namespace {
template<class Tag> [[nodiscard]] bool nonzero(const identity::IdentityId<Tag>& value) noexcept {
    std::uint8_t nonzero_byte = 0;
    for (auto b : value.bytes) nonzero_byte = static_cast<std::uint8_t>(nonzero_byte | b);
    return nonzero_byte != 0;
}
[[nodiscard]] bool valid_scope(const Scope& s) noexcept {
    return nonzero(s.community) && nonzero(s.membership) && nonzero(s.resource);
}
[[nodiscard]] bool valid_rule(const Rule& r) noexcept {
    if (!valid_scope(r.scope) || r.expires_at <= r.not_before || r.expires_at == 0) return false;
    if ((r.action_mask & ~known_action_mask) != 0) return false;
    // Ban is a deny-only entry. A ban carrying grant bits is ambiguous.
    return r.banned ? r.action_mask == 0 : r.action_mask != 0;
}
[[nodiscard]] std::uint8_t action_bit(Action action) noexcept {
    switch (action) {
        case Action::read_channel: return read_mask;
        case Action::post_message: return post_mask;
    }
    return 0;
}
} // namespace

Mutation LocalPolicyTable::upsert(const Rule& rule) noexcept {
    if (!valid_scope(rule.scope)) return Mutation::invalid_scope;
    if (!valid_rule(rule)) return Mutation::invalid_rule;
    for (std::size_t i = 0; i < size_; ++i) {
        if (rules_[i].scope == rule.scope) {
            if (rules_[i] == rule) return Mutation::unchanged;
            if (epoch_ == std::numeric_limits<std::uint64_t>::max()) return Mutation::epoch_exhausted;
            rules_[i] = rule;
            ++epoch_;
            return Mutation::applied;
        }
    }
    if (size_ == maximum_local_rules) return Mutation::capacity;
    if (epoch_ == std::numeric_limits<std::uint64_t>::max()) return Mutation::epoch_exhausted;
    rules_[size_++] = rule;
    ++epoch_;
    return Mutation::applied;
}

Mutation LocalPolicyTable::erase(const Scope& scope) noexcept {
    if (!valid_scope(scope)) return Mutation::invalid_scope;
    for (std::size_t i = 0; i < size_; ++i) {
        if (rules_[i].scope == scope) {
            if (epoch_ == std::numeric_limits<std::uint64_t>::max()) return Mutation::epoch_exhausted;
            // Discard the revoked entry and its old rule data (no tombstone).
            for (std::size_t j = i + 1; j < size_; ++j) rules_[j - 1] = rules_[j];
            rules_[--size_] = Rule{};
            ++epoch_;
            return Mutation::applied;
        }
    }
    return Mutation::unchanged;
}

Mutation LocalPolicyTable::clear() noexcept {
    if (!size_) return Mutation::unchanged;
    if (epoch_ == std::numeric_limits<std::uint64_t>::max()) return Mutation::epoch_exhausted;
    for (std::size_t i = 0; i < size_; ++i) rules_[i] = Rule{};
    size_ = 0;
    ++epoch_;
    return Mutation::applied;
}

Observation LocalPolicyTable::inspect(const Scope& scope, Action action,
                                      std::uint64_t trusted_utc_seconds) const noexcept {
    Observation result{epoch_, Candidate::not_listed};
    const auto bit = action_bit(action);
    if (!valid_scope(scope) || bit == 0 || trusted_utc_seconds == 0) return result;
    for (std::size_t i = 0; i < size_; ++i) {
        const auto& rule = rules_[i];
        if (rule.scope == scope) {
            if (!rule.banned && rule.not_before <= trusted_utc_seconds &&
                trusted_utc_seconds < rule.expires_at && (rule.action_mask & bit) != 0)
                result.candidate = Candidate::listed;
            return result;
        }
    }
    return result;
}

Candidate LocalPolicyTable::recheck(const Observation& observed, const Scope& scope,
                                    Action action, std::uint64_t trusted_utc_seconds) const noexcept {
    if (observed.candidate != Candidate::listed || observed.epoch == 0 || observed.epoch != epoch_)
        return Candidate::not_listed;
    return inspect(scope, action, trusted_utc_seconds).candidate;
}

} // namespace ghostwire::permissions
