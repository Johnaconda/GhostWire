#pragma once
#include "ghostwire/core/crypto_service.hpp"
#include "ghostwire/identity/identity_service.hpp"

namespace ghostwire::composition {
inline constexpr core::ServiceId identity_service{3};

// Composition-layer lifecycle adapter: core never links or looks up identity.
// The owning application must register cryptographic_provider_service as this
// service's dependency, and keep its injected CryptoService alive until cleanup.
class IdentityServiceAdapter final : public core::Service {
  public:
    explicit IdentityServiceAdapter(core::CryptoService &crypto, identity::ServiceLimits limits = {})
        : crypto_(crypto), limits_(limits) {}
    core::Result<void> start(const core::ApplicationContext &) override;
    core::Result<void> stop(const core::ShutdownContext &) override;
    // Owner-thread borrow only while running; null before start/after cleanup.
    [[nodiscard]] identity::IdentityService *identity() noexcept { return identity_.get(); }

  private:
    core::CryptoService &crypto_;
    const identity::ServiceLimits limits_;
    // One logical active service reservation, not CPU or resident-memory
    // measurement. Declared bytes cover bounded retained public frames and
    // 32-byte private seeds; provider/workspace/allocator costs remain separate.
    core::WorkLease reservation_;
    std::unique_ptr<identity::IdentityService> identity_;
    bool started_ = false;
};
} // namespace ghostwire::composition
