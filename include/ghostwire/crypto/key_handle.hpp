#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <utility>

namespace ghostwire::crypto {
namespace detail {
struct SecretAllocation;
struct ProviderState;
} // namespace detail
class CryptoProvider;

// Borrowed bytes must not outlive this object, a move, or reset(). The caller
// controls any explicit copies. No implicit conversion, serializer or logger.
class SecretBuffer final {
  public:
    SecretBuffer() noexcept;
    ~SecretBuffer() noexcept;
    SecretBuffer(SecretBuffer &&) noexcept;
    SecretBuffer &operator=(SecretBuffer &&) noexcept;
    SecretBuffer(const SecretBuffer &) = delete;
    SecretBuffer &operator=(const SecretBuffer &) = delete;
    [[nodiscard]] std::span<const std::uint8_t> borrow() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    void reset() noexcept;

  private:
    explicit SecretBuffer(std::unique_ptr<detail::SecretAllocation>) noexcept;
    std::unique_ptr<detail::SecretAllocation> storage_;
    friend class CryptoProvider;
    friend struct detail::ProviderState;
};

class AeadKey final {
  public:
    AeadKey() noexcept = default;
    AeadKey(AeadKey &&) noexcept = default;
    AeadKey &operator=(AeadKey &&) noexcept = default;
    AeadKey(const AeadKey &) = delete;
    AeadKey &operator=(const AeadKey &) = delete;
    [[nodiscard]] bool valid() const noexcept { return secret_.size() == 32; }
    void reset() noexcept { secret_.reset(); }

  private:
    explicit AeadKey(SecretBuffer secret) noexcept : secret_(std::move(secret)) {}
    SecretBuffer secret_;
    friend class CryptoProvider;
};

class SigningKey final {
  public:
    SigningKey() noexcept = default;
    SigningKey(SigningKey &&) noexcept = default;
    SigningKey &operator=(SigningKey &&) noexcept = default;
    SigningKey(const SigningKey &) = delete;
    SigningKey &operator=(const SigningKey &) = delete;
    [[nodiscard]] bool valid() const noexcept { return secret_.size() == 32; }
    void reset() noexcept { secret_.reset(); }

  private:
    explicit SigningKey(SecretBuffer secret) noexcept : secret_(std::move(secret)) {}
    SecretBuffer secret_;
    friend class CryptoProvider;
};
} // namespace ghostwire::crypto
