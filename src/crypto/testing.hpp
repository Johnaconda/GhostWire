#pragma once
#ifndef GW_CRYPTO_TESTING
#error "Crypto failure injection is only available in the separate test library"
#endif
#include "ghostwire/crypto/crypto_provider.hpp"
#include <atomic>
#include <functional>

namespace ghostwire::crypto::detail {
// Owned by the test provider and its allocations; no process-global hooks.
// Callbacks run inside an operation or immediately after clearing, before free.
// Tests must not mutate callback registrations concurrently; on_clear must not throw.
struct TestHooks {
    std::atomic<bool> fail_initialization{false}, fail_entropy{false}, fail_allocation{false};
    std::function<void()> after_entropy;
    std::function<void(std::span<const std::uint8_t>)> on_clear;
};
struct TestAccess {
    static Result<std::unique_ptr<CryptoProvider>> create(std::shared_ptr<TestHooks>, Limits = {},
                                                          std::stop_token = {});
};
} // namespace ghostwire::crypto::detail
