#include "ghostwire/core/crypto_service.hpp"

namespace ghostwire::core {
Result<void> CryptoService::start(const ApplicationContext &context) {
    if (started_)
        return Error{ErrorCode::internal, "Crypto lifecycle cannot be restarted.",
                     ErrorReason::invalid_state};
    started_ = true;
    auto created = crypto::CryptoProvider::create(limits_, context.stop_token());
    if (!created)
        return Error{ErrorCode::internal, "Cryptographic provider initialization failed.",
                     created.error().code == crypto::ErrorCode::cancelled ? ErrorReason::cancelled
                                                                          : ErrorReason::unavailable};
    provider_ = std::move(created).value();
    return {};
}
Result<void> CryptoService::stop(const ShutdownContext &) {
    started_ = true; // Terminal even when cleanup precedes startup.
    if (provider_ && !provider_->try_close())
        return Error{ErrorCode::internal, "Cryptographic work has not finished.",
                     ErrorReason::outstanding_work};
    provider_.reset();
    return {};
}
} // namespace ghostwire::core
