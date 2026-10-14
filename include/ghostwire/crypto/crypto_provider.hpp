#pragma once
#include "ghostwire/crypto/crypto_error.hpp"
#include "ghostwire/crypto/key_handle.hpp"
#include <array>
#include <stop_token>
#include <string_view>
#include <vector>

namespace ghostwire::crypto {
namespace detail {
struct TestAccess;
}
inline constexpr std::size_t aead_key_bytes = 32, aead_nonce_bytes = 12, aead_tag_bytes = 16;
inline constexpr std::size_t signing_seed_bytes = 32, public_key_bytes = 32, signature_bytes = 64;
inline constexpr std::size_t maximum_input_bytes = 1024 * 1024;
using PublicKey = std::array<std::uint8_t, public_key_bytes>;
using Signature = std::array<std::uint8_t, signature_bytes>;
using Digest = std::array<std::uint8_t, 32>;

struct Limits {
    std::size_t message_bytes = 64 * 1024; // Also applies to associated data independently.
    std::size_t random_bytes = 64 * 1024;
    std::size_t live_secret_bytes = 4 * 1024 * 1024;
    std::size_t live_secret_handles = 256;
    std::size_t workspace_bytes = 3 * 1024 * 1024; // Declared work, not OS RSS.
};
struct Diagnostic {
    std::size_t live_secret_bytes = 0, live_secret_handles = 0;
    bool stopped = false;
    // Botan's locked pool is best effort; individual allocations are not certified locked.
    bool all_secret_memory_locked = false;
    bool persistence_available = false;
};

// Explicit owner. Concurrent operations fail immediately with busy; no worker or
// queue. request_stop()/diagnostic() may race operations. Destruction may not.
// All operations check owned, application and per-call cancellation before/after
// bounded provider work; OS RNG calls are not forcibly interruptible.
class CryptoProvider final {
  public:
    static Result<std::unique_ptr<CryptoProvider>> create(Limits limits = {},
                                                          std::stop_token application_stop = {});
    ~CryptoProvider() noexcept;
    CryptoProvider(const CryptoProvider &) = delete;
    CryptoProvider &operator=(const CryptoProvider &) = delete;
    [[nodiscard]] static constexpr std::string_view backend_version() noexcept { return "Botan 3.13.0"; }
    void request_stop() noexcept;
    // Terminal stop. Returns false if a call still owns the admission lock; owner
    // must retain the provider and retry after dependents finish. Never waits.
    // A true result does not authorize destruction racing other method callers;
    // join all consumer workers before destroying the owner.
    [[nodiscard]] bool try_close() noexcept;
    [[nodiscard]] Diagnostic diagnostic() const;
    Result<SecretBuffer> random(std::size_t bytes, std::stop_token = {});
    Result<AeadKey> generate_aead_key(std::stop_token = {});
    Result<SigningKey> generate_signing_key(std::stop_token = {});
    // Input is copied into clearing storage; caller still owns and must clear its copy.
    Result<AeadKey> import_aead_key(std::span<const std::uint8_t>, std::stop_token = {});
    Result<SigningKey> import_signing_seed(std::span<const std::uint8_t>, std::stop_token = {});
    Result<PublicKey> public_key(const SigningKey &, std::stop_token = {});
    // RFC 8439: caller MUST guarantee nonce uniqueness for each encryption under
    // a key. Ciphertext result is payload || 16-byte tag; nonce/AAD are external.
    Result<std::vector<std::uint8_t>> encrypt(const AeadKey &, std::span<const std::uint8_t> nonce,
                                              std::span<const std::uint8_t> plaintext,
                                              std::span<const std::uint8_t> associated_data = {},
                                              std::stop_token = {});
    // No caller-visible plaintext exists until authentication and cancellation checks pass.
    Result<SecretBuffer> decrypt(const AeadKey &, std::span<const std::uint8_t> nonce,
                                 std::span<const std::uint8_t> ciphertext_and_tag,
                                 std::span<const std::uint8_t> associated_data = {}, std::stop_token = {});
    // Pure Ed25519 over PUBLIC bytes. Botan buffers this message in ordinary heap
    // memory; these calls do not promise confidentiality of the signed message.
    Result<Signature> sign_public(const SigningKey &, std::span<const std::uint8_t> public_message,
                                  std::stop_token = {});
    Result<void> verify_public(std::span<const std::uint8_t> public_key,
                               std::span<const std::uint8_t> public_message,
                               std::span<const std::uint8_t> signature, std::stop_token = {});
    Result<Digest> sha256(std::span<const std::uint8_t>, std::stop_token = {});

  private:
    explicit CryptoProvider(std::unique_ptr<detail::ProviderState>) noexcept;
    static Result<std::unique_ptr<CryptoProvider>> from_state(std::unique_ptr<detail::ProviderState>);
    std::unique_ptr<detail::ProviderState> state_;
    friend struct detail::TestAccess;
};
} // namespace ghostwire::crypto
