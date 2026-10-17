#pragma once
#include "ghostwire/crypto/crypto_provider.hpp"
#include "ghostwire/identity/identity_error.hpp"
#include <array>
#include <compare>
#include <cstdint>

namespace ghostwire::identity {
// Public values only. Different tags cannot implicitly convert or compare.
// These hashes identify keys and scopes; they never prove key possession.
template <typename Tag> struct IdentityId {
    std::array<std::uint8_t, 32> bytes{};
    auto operator<=>(const IdentityId &) const = default;
};
struct ProfileIdentityTag;
struct CommunityTag;
struct MembershipTag;
struct ProfileDeviceTag;
struct MembershipDeviceTag;
using ProfileIdentityId = IdentityId<ProfileIdentityTag>;
using CommunityId = IdentityId<CommunityTag>;
using MembershipId = IdentityId<MembershipTag>;
using ProfileDeviceId = IdentityId<ProfileDeviceTag>;
using MembershipDeviceId = IdentityId<MembershipDeviceTag>;

Result<ProfileIdentityId> derive_profile_id(crypto::CryptoProvider &, const crypto::PublicKey &,
                                            std::stop_token = {});
Result<CommunityId> derive_community_id(crypto::CryptoProvider &, const crypto::PublicKey &,
                                        std::stop_token = {});
Result<MembershipId> derive_membership_id(crypto::CryptoProvider &, const CommunityId &,
                                          const crypto::PublicKey &, std::stop_token = {});
Result<ProfileDeviceId> derive_profile_device_id(crypto::CryptoProvider &, const ProfileIdentityId &,
                                                 const crypto::PublicKey &, std::stop_token = {});
Result<MembershipDeviceId> derive_membership_device_id(crypto::CryptoProvider &, const MembershipId &,
                                                       const crypto::PublicKey &, std::stop_token = {});
} // namespace ghostwire::identity
