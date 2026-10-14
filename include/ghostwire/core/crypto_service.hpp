#pragma once
#include "ghostwire/core/service.hpp"
#include "ghostwire/crypto/crypto_provider.hpp"

namespace ghostwire::core {
inline constexpr ServiceId cryptographic_provider_service{2};
// Root explicitly registers/owns this adapter. Consumers borrow the service by
// constructor injection and declare its ID as a lifecycle dependency. No lookup.
class CryptoService final : public Service {
  public:
    explicit CryptoService(crypto::Limits limits = {}) : limits_(limits) {}
    Result<void> start(const ApplicationContext &) override;
    Result<void> stop(const ShutdownContext &) override;
    // Null before successful start and after successful stop. Owner-thread access;
    // a consumer may borrow for calls only while it keeps this dependency alive.
    [[nodiscard]] crypto::CryptoProvider *provider() noexcept { return provider_.get(); }

  private:
    const crypto::Limits limits_;
    std::unique_ptr<crypto::CryptoProvider> provider_;
    bool started_ = false;
};
} // namespace ghostwire::core
