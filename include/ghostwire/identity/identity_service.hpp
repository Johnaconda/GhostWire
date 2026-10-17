#pragma once
#include "ghostwire/core/clock.hpp"
#include "ghostwire/identity/identity_record.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ghostwire::identity {
namespace detail {
struct ServiceState;
struct OwnerIdentity;
struct ProfileTag;
struct MembershipTag;
struct TrustTag;
} // namespace detail
// Process-local capabilities, never identities or serializable record fields.
// A token retains only an empty owner identity, not the service or its keys.
template <class Tag> class LocalHandle final {
  public:
    LocalHandle() = default;
    bool operator==(const LocalHandle &) const = default;

  private:
    std::shared_ptr<const detail::OwnerIdentity> owner_;
    std::size_t slot_ = 0, parent_ = 0;
    std::uint64_t generation_ = 0, parent_generation_ = 0;
    friend struct detail::ServiceState;
    friend class IdentityService;
};
using ProfileHandle = LocalHandle<detail::ProfileTag>;
using MembershipHandle = LocalHandle<detail::MembershipTag>;
using TrustHandle = LocalHandle<detail::TrustTag>;

struct ServiceLimits {
    // Counts/bytes may be lowered, not raised above these bounded P02 ceilings.
    std::size_t profiles = 8, devices_per_authority = 32, memberships_per_profile = 32;
    std::size_t private_keys = 128, trust_contexts = 256, public_record_bytes = 2 * 1024 * 1024;
    std::size_t input_bytes = 8 * 1024;
    std::uint64_t issued_lifetime_seconds = 30 * 24 * 60 * 60;
    std::uint64_t maximum_lifetime_seconds = 90 * 24 * 60 * 60;
    std::chrono::milliseconds operation_timeout{100};
};
struct Labels {
    std::string alias;
    std::string display_name;
    bool operator==(const Labels &) const = default;
};
struct ProfileDevice {
    ProfileDeviceId id;
    crypto::PublicKey public_key{};
};
struct MembershipDevice {
    MembershipDeviceId id;
    crypto::PublicKey public_key{};
};
struct ProfileSnapshot {
    ProfileHandle handle;
    ProfileIdentityId id;
    crypto::PublicKey public_key{};
    Labels labels;
    std::vector<ProfileDevice> devices;
    std::uint64_t revision = 0;
};
struct MembershipSnapshot {
    MembershipHandle handle;
    MembershipId id;
    CommunityId community;
    crypto::PublicKey public_key{};
    Labels labels;
    std::vector<MembershipDevice> devices;
    std::uint64_t revision = 0;
};
enum class TrustState { unknown, pinned, independently_verified, blocked, conflict };
struct TrustSnapshot {
    TrustHandle handle;
    ExpectedAuthority authority;
    TrustState state = TrustState::unknown;
    std::uint64_t highest_revision = 0;
    bool has_record = false;
};
enum class ServiceState { running, stopping, stopped };
struct ServiceDiagnostic {
    ServiceState state = ServiceState::stopped;
    std::size_t profiles = 0, memberships = 0, devices = 0, private_keys = 0;
    std::size_t trust_contexts = 0, public_record_bytes = 0;
    bool persistence_available = false;
};

// Explicitly owned, volatile service. Provider and monotonic clock outlive it.
// All calls except request_stop are owner-thread calls; join consumers before
// destruction. Calls reject reentrancy as busy; there are no workers or queues.
// Deadlines are cooperative and cannot preempt an OS entropy/provider operation.
// The trusted caller supplies UTC separately; no system wall-clock lookup occurs.
class IdentityService final {
  public:
    static Result<std::unique_ptr<IdentityService>> create(crypto::CryptoProvider &, const core::Clock &,
                                                           ServiceLimits = {}, std::stop_token = {});
    ~IdentityService() noexcept;
    IdentityService(const IdentityService &) = delete;
    IdentityService &operator=(const IdentityService &) = delete;
    void request_stop() noexcept;
    Result<void> close(); // Idempotent; clears only this service's volatile state.
    [[nodiscard]] ServiceDiagnostic diagnostic() const noexcept;

    Result<ProfileHandle> create_profile(std::string_view alias, std::string_view display_name,
                                         VerificationTime, std::stop_token = {});
    Result<ProfileSnapshot> profile(ProfileHandle, std::stop_token = {});
    Result<std::vector<ProfileSnapshot>> profiles(std::stop_token = {});
    Result<void> rename_profile(ProfileHandle, std::string_view alias, std::string_view display_name,
                                std::stop_token = {});
    Result<void> delete_profile(ProfileHandle, std::stop_token = {});
    Result<CommunityId> pin_community(ProfileHandle, const crypto::PublicKey &, std::stop_token = {});
    Result<void> forget_community(ProfileHandle, CommunityId, std::stop_token = {});
    // This prepares independent credentials. It does not enroll/admit a member.
    Result<MembershipHandle> prepare_membership(ProfileHandle, CommunityId, std::string_view alias,
                                                std::string_view display_name, VerificationTime,
                                                std::stop_token = {});
    Result<MembershipSnapshot> membership(ProfileHandle, MembershipHandle, std::stop_token = {});
    Result<std::vector<MembershipSnapshot>> memberships(ProfileHandle, std::stop_token = {});
    Result<std::vector<MembershipSnapshot>> find_memberships(ProfileHandle, CommunityId,
                                                             std::string_view alias, std::stop_token = {});
    Result<void> rename_membership(ProfileHandle, MembershipHandle, std::string_view alias,
                                   std::string_view display_name, std::stop_token = {});
    Result<void> delete_membership(ProfileHandle, MembershipHandle, std::stop_token = {});

    Result<DeviceSetFrame> profile_record(ProfileHandle, std::stop_token = {});
    Result<DeviceSetFrame> membership_record(ProfileHandle, MembershipHandle, std::stop_token = {});
    Result<ProfileDeviceId> add_profile_device(ProfileHandle, VerificationTime, std::stop_token = {});
    Result<MembershipDeviceId> add_membership_device(ProfileHandle, MembershipHandle, VerificationTime,
                                                     std::stop_token = {});
    Result<ProfileDeviceId> rotate_profile_device(ProfileHandle, ProfileDeviceId, VerificationTime,
                                                  std::stop_token = {});
    Result<MembershipDeviceId> rotate_membership_device(ProfileHandle, MembershipHandle, MembershipDeviceId,
                                                        VerificationTime, std::stop_token = {});
    Result<void> revoke_profile_device(ProfileHandle, ProfileDeviceId, VerificationTime,
                                       std::stop_token = {});
    Result<void> revoke_membership_device(ProfileHandle, MembershipHandle, MembershipDeviceId,
                                          VerificationTime, std::stop_token = {});
    Result<void> renew_profile_record(ProfileHandle, VerificationTime, std::stop_token = {});
    Result<void> renew_membership_record(ProfileHandle, MembershipHandle, VerificationTime,
                                         std::stop_token = {});

    // Trusted local-owner operations, never inferred from signatures or aliases.
    Result<TrustHandle> pin_authority(ProfileHandle, ExpectedAuthority, std::stop_token = {});
    Result<TrustSnapshot> trust(ProfileHandle, TrustHandle, std::stop_token = {});
    Result<void> mark_independently_verified(ProfileHandle, TrustHandle, std::stop_token = {});
    Result<void> set_blocked(ProfileHandle, TrustHandle, bool blocked, std::stop_token = {});
    Result<void> forget_authority(ProfileHandle, TrustHandle, std::stop_token = {});
    Result<DeviceSet> verify_record(std::span<const std::uint8_t>, const ExpectedAuthority &,
                                    VerificationTime, std::stop_token = {});
    Result<void> accept_record(ProfileHandle, TrustHandle, std::span<const std::uint8_t>, VerificationTime,
                               std::stop_token = {});
    Result<void> resolve_conflict(ProfileHandle, TrustHandle, std::span<const std::uint8_t>, VerificationTime,
                                  std::stop_token = {});
    // Local recognition only, never live possession or an action permission.
    Result<void> recognize_device(ProfileHandle, TrustHandle, const crypto::PublicKey &, VerificationTime,
                                  std::stop_token = {});
    Result<void> revalidate_time(VerificationTime, std::stop_token = {});

  private:
    explicit IdentityService(std::unique_ptr<detail::ServiceState>) noexcept;
    std::unique_ptr<detail::ServiceState> state_;
};
} // namespace ghostwire::identity
