#pragma once
// HF19 ownership: permissions-local-policy. Pure local policy proposals only.
// TODO(AUTH-PRE): composition-owned policy mutation, reviewed session verifier,
// channel binding and atomic resource-owner commit gate remain unimplemented.
// Never treat a Candidate as an authorization token or channel privilege.
#include "ghostwire/identity/identity_id.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace ghostwire::permissions {

// Resource identifier is typed, exact 32 bytes, and never derived from an alias.
struct ResourceTag;
using ResourceId = identity::IdentityId<ResourceTag>;

struct Scope final {
    identity::CommunityId community{};
    identity::MembershipId membership{};
    ResourceId resource{};
    bool operator==(const Scope&) const = default;
};

enum class Action : std::uint8_t { read_channel = 1, post_message = 2 };
enum class Mutation : std::uint8_t {
    applied, unchanged, invalid_scope, invalid_rule, capacity, epoch_exhausted
};
enum class Candidate : std::uint8_t { not_listed, listed };
inline constexpr std::uint8_t read_mask = 1U, post_mask = 2U;
inline constexpr std::uint8_t known_action_mask = read_mask | post_mask;
inline constexpr std::size_t maximum_local_rules = 64;

struct Rule final {
    Scope scope{};
    std::uint8_t action_mask = 0;
    std::uint64_t not_before = 0;
    std::uint64_t expires_at = 0; // exclusive UTC boundary
    bool banned = false;
    bool operator==(const Rule&) const = default;
};

// Public, forgeable observation used only to detect stale local policy reads.
// NEVER a remote bearer capability or proof of authenticated device possession.
struct Observation final {
    std::uint64_t epoch = 0;
    Candidate candidate = Candidate::not_listed;
};

// Owned on a single composition/resource-owner thread; caller must serialize
// all methods and any actual action commit. No storage, locks, workers, keys or
// sockets. A listed candidate is NOT a final permission decision.
class LocalPolicyTable final {
  public:
    [[nodiscard]] Mutation upsert(const Rule& rule) noexcept;
    [[nodiscard]] Mutation erase(const Scope& scope) noexcept;
    [[nodiscard]] Mutation clear() noexcept;
    [[nodiscard]] Observation inspect(const Scope& scope, Action action,
                                      std::uint64_t trusted_utc_seconds) const noexcept;
    [[nodiscard]] Candidate recheck(const Observation& observed, const Scope& scope,
                                    Action action, std::uint64_t trusted_utc_seconds) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] std::uint64_t epoch() const noexcept { return epoch_; }

  private:
    std::array<Rule, maximum_local_rules> rules_{};
    std::size_t size_ = 0;
    std::uint64_t epoch_ = 1; // zero is never a valid policy observation.
};

} // namespace ghostwire::permissions
