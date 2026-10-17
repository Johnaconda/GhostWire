#include "ghostwire/composition/identity_service_adapter.hpp"
#include <limits>

namespace ghostwire::composition {
namespace {
core::ErrorReason reason(identity::ErrorCode code) noexcept {
    switch (code) {
    case identity::ErrorCode::cancelled:
        return core::ErrorReason::cancelled;
    case identity::ErrorCode::resource_exhausted:
        return core::ErrorReason::resource_exhausted;
    case identity::ErrorCode::busy:
        return core::ErrorReason::outstanding_work;
    case identity::ErrorCode::deadline_exceeded:
        return core::ErrorReason::deadline_exceeded;
    default:
        return core::ErrorReason::invalid_state;
    }
}
} // namespace

core::Result<void> IdentityServiceAdapter::start(const core::ApplicationContext &context) {
    if (started_)
        return core::Error{core::ErrorCode::internal, "Identity lifecycle cannot be restarted.",
                           core::ErrorReason::invalid_state};
    started_ = true;
    auto *provider = crypto_.provider();
    if (!provider)
        return core::Error{core::ErrorCode::internal, "Identity requires a running crypto dependency.",
                           core::ErrorReason::missing_dependency};
    if (limits_.private_keys > (std::numeric_limits<std::size_t>::max() - limits_.public_record_bytes) / 32)
        return core::Error{core::ErrorCode::internal, "Identity reservation exceeds its budget.",
                           core::ErrorReason::invalid_budget};
    const auto bytes = limits_.public_record_bytes + limits_.private_keys * 32;
    auto reservation = context.admission().try_acquire(core::WorkKind::active, bytes, context.stop_token());
    if (!reservation)
        return reservation.error();
    auto created =
        identity::IdentityService::create(*provider, context.clock(), limits_, context.stop_token());
    if (!created)
        return core::Error{core::ErrorCode::internal, "Identity service initialization failed.",
                           reason(created.error().code)};
    reservation_ = std::move(reservation).value();
    identity_ = std::move(created).value();
    return {};
}

core::Result<void> IdentityServiceAdapter::stop(const core::ShutdownContext &) {
    started_ = true;
    if (identity_) {
        identity_->request_stop();
        auto closed = identity_->close();
        if (!closed)
            return core::Error{core::ErrorCode::internal, "Identity cleanup has not completed.",
                               reason(closed.error().code)};
        identity_.reset();
    }
    reservation_.release();
    return {};
}
} // namespace ghostwire::composition
