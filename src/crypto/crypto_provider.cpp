#include "ghostwire/crypto/crypto_provider.hpp"
#include <algorithm>
#include <botan_all.h>
#include <mutex>
#include <new>
#include <type_traits>
#ifdef GW_CRYPTO_TESTING
#include "testing.hpp"
#endif

static_assert(BOTAN_VERSION_MAJOR == 3 && BOTAN_VERSION_MINOR == 13 && BOTAN_VERSION_PATCH == 0);
#if !defined(BOTAN_HAS_SYSTEM_RNG) || !defined(BOTAN_HAS_AEAD_CHACHA20_POLY1305) ||                          \
    !defined(BOTAN_HAS_ED25519) || !defined(BOTAN_HAS_SHA2_32)
#error "The reviewed Botan primitive/memory subset is required"
#endif

namespace ghostwire::crypto {
namespace detail {
namespace {
constexpr std::size_t workspace_overhead = 64 * 1024;
constexpr std::size_t maximum_budget = 64 * 1024 * 1024;
struct Failure {
    ErrorCode code;
};
bool valid_limits(const Limits &v) noexcept {
    return v.message_bytes <= maximum_input_bytes && v.random_bytes > 0 &&
           v.random_bytes <= maximum_input_bytes && v.live_secret_bytes > 0 &&
           v.live_secret_bytes <= maximum_budget && v.live_secret_handles > 0 &&
           v.live_secret_handles <= 4096 && v.workspace_bytes >= workspace_overhead &&
           v.workspace_bytes <= maximum_budget;
}
} // namespace

struct Accounting {
    explicit Accounting(Limits value) : limits(value) {}
    const Limits limits;
    std::mutex mutex;
    std::size_t bytes = 0, handles = 0;
#ifdef GW_CRYPTO_TESTING
    std::shared_ptr<TestHooks> hooks;
#endif
    void release(std::size_t size) noexcept {
        const std::lock_guard lock(mutex);
        bytes -= size;
        --handles;
    }
};

struct SecretAllocation {
    SecretAllocation(std::shared_ptr<Accounting> owner, std::size_t count)
        : accounting(std::move(owner)), size(count) {
#ifdef GW_CRYPTO_TESTING
        if (accounting->hooks && accounting->hooks->fail_allocation.load())
            throw std::bad_alloc();
#endif
        data = static_cast<std::uint8_t *>(Botan::allocate_memory(size, 1));
    }
    ~SecretAllocation() noexcept {
        Botan::secure_scrub_memory(data, size);
#ifdef GW_CRYPTO_TESTING
        if (accounting->hooks && accounting->hooks->on_clear)
            accounting->hooks->on_clear({data, size});
#endif
        Botan::deallocate_memory(data, size, 1);
        accounting->release(size);
    }
    SecretAllocation(const SecretAllocation &) = delete;
    SecretAllocation &operator=(const SecretAllocation &) = delete;
    std::shared_ptr<Accounting> accounting;
    std::size_t size;
    std::uint8_t *data = nullptr; // Owned only here; Botan's matching clearing allocator/free.
};

struct ProviderState {
    ProviderState(Limits limits, std::stop_token application)
        : accounting(std::make_shared<Accounting>(limits)), application_stop(application) {}
    std::shared_ptr<Accounting> accounting;
    std::stop_source stop;
    std::stop_token application_stop;
    std::mutex operation;
    Botan::System_RNG rng;

    bool cancelled(std::stop_token token) const noexcept {
        return stop.stop_requested() || application_stop.stop_requested() || token.stop_requested();
    }
    void check_cancel(std::stop_token token) const {
        if (cancelled(token))
            throw Failure{ErrorCode::cancelled};
    }
    void check_message(std::size_t size) const {
        if (size > accounting->limits.message_bytes)
            throw Failure{ErrorCode::invalid_size};
    }
    void workspace(std::size_t size) const {
        // Called only after input caps. Arithmetic cannot exceed a few MiB.
        if (size > accounting->limits.workspace_bytes)
            throw Failure{ErrorCode::resource_exhausted};
    }
    SecretBuffer allocate(std::size_t bytes) {
        if (bytes == 0)
            return {};
        {
            const std::lock_guard lock(accounting->mutex);
            if (bytes > accounting->limits.live_secret_bytes - accounting->bytes ||
                accounting->handles >= accounting->limits.live_secret_handles)
                throw Failure{ErrorCode::resource_exhausted};
            accounting->bytes += bytes;
            ++accounting->handles;
        }
        try {
            return SecretBuffer(std::make_unique<SecretAllocation>(accounting, bytes));
        } catch (...) {
            accounting->release(bytes);
            throw;
        }
    }
    SecretBuffer copy(std::span<const std::uint8_t> input) {
        auto output = allocate(input.size());
        if (!input.empty())
            std::copy(input.begin(), input.end(), output.storage_->data);
        return output;
    }
    void entropy(std::span<std::uint8_t> output, std::stop_token token) {
        try {
#ifdef GW_CRYPTO_TESTING
            if (accounting->hooks && accounting->hooks->fail_entropy.load()) {
                // Deliberately dirty a partial result to exercise failure cleanup.
                if (!output.empty())
                    output.front() = 0xa5;
                throw Botan::System_Error("Synthetic entropy failure with private detail");
            }
#endif
            while (!output.empty()) {
                check_cancel(token);
                const auto count = std::min<std::size_t>(output.size(), 4096);
                rng.randomize(output.first(count));
                output = output.subspan(count);
            }
#ifdef GW_CRYPTO_TESTING
            if (accounting->hooks && accounting->hooks->after_entropy)
                accounting->hooks->after_entropy();
#endif
        } catch (const Botan::System_Error &) {
            throw Failure{ErrorCode::entropy_unavailable};
        }
        check_cancel(token);
    }
    SecretBuffer generate(std::size_t bytes, std::stop_token token) {
        auto output = allocate(bytes);
        if (bytes != 0)
            entropy({output.storage_->data, bytes}, token);
        return output;
    }
    template <typename T, typename F> Result<T> run(Operation id, std::stop_token token, F &&action) {
        if (cancelled(token))
            return Error{ErrorCode::cancelled, id};
        const std::unique_lock lock(operation, std::try_to_lock);
        if (!lock.owns_lock())
            return Error{ErrorCode::busy, id};
        try {
            check_cancel(token);
            if constexpr (std::is_void_v<T>) {
                action();
                check_cancel(token);
                return {};
            } else {
                T output = action();
                check_cancel(token);
                return output;
            }
        } catch (const Failure &failure) {
            return Error{failure.code, id};
        } catch (const std::bad_alloc &) {
            return Error{ErrorCode::allocation_failed, id};
        } catch (const Botan::Invalid_Authentication_Tag &) {
            return Error{ErrorCode::authentication_failed, id};
        } catch (...) {
            // Exception messages may include private data; never forward them.
            return Error{id == Operation::initialize ? ErrorCode::initialization_failed
                                                     : ErrorCode::provider_failure,
                         id};
        }
    }
};
} // namespace detail

SecretBuffer::SecretBuffer() noexcept = default;
SecretBuffer::SecretBuffer(std::unique_ptr<detail::SecretAllocation> value) noexcept
    : storage_(std::move(value)) {}
SecretBuffer::~SecretBuffer() noexcept = default;
SecretBuffer::SecretBuffer(SecretBuffer &&) noexcept = default;
SecretBuffer &SecretBuffer::operator=(SecretBuffer &&) noexcept = default;
std::span<const std::uint8_t> SecretBuffer::borrow() const noexcept {
    return storage_ ? std::span<const std::uint8_t>{storage_->data, storage_->size}
                    : std::span<const std::uint8_t>{};
}
std::size_t SecretBuffer::size() const noexcept {
    return storage_ ? storage_->size : 0;
}
void SecretBuffer::reset() noexcept {
    storage_.reset();
}

CryptoProvider::CryptoProvider(std::unique_ptr<detail::ProviderState> state) noexcept
    : state_(std::move(state)) {}
CryptoProvider::~CryptoProvider() noexcept {
    request_stop();
}
Result<std::unique_ptr<CryptoProvider>>
CryptoProvider::from_state(std::unique_ptr<detail::ProviderState> state) {
    if (!detail::valid_limits(state->accounting->limits))
        return Error{ErrorCode::invalid_limits, Operation::initialize};
    auto initialized = state->run<void>(Operation::initialize, {}, [&] {
#ifdef GW_CRYPTO_TESTING
        if (state->accounting->hooks && state->accounting->hooks->fail_initialization.load())
            throw detail::Failure{ErrorCode::initialization_failed};
        if (state->accounting->hooks && state->accounting->hooks->fail_allocation.load())
            throw std::bad_alloc();
#endif
        if (Botan::version_major() != 3 || Botan::version_minor() != 13 || Botan::version_patch() != 0)
            throw detail::Failure{ErrorCode::initialization_failed};
        Botan::initialize_allocator();
        // Check actual entropy and availability without generating an identity.
        Botan::secure_vector<std::uint8_t> probe(32);
        state->entropy(probe, {});
        auto hash = Botan::HashFunction::create_or_throw("SHA-256");
        auto aead = Botan::AEAD_Mode::create_or_throw("ChaCha20Poly1305", Botan::Cipher_Dir::Encryption);
        if (hash->output_length() != 32 || aead->tag_size() != aead_tag_bytes)
            throw detail::Failure{ErrorCode::initialization_failed};
    });
    if (!initialized)
        return initialized.error();
    return std::unique_ptr<CryptoProvider>(new CryptoProvider(std::move(state)));
}
Result<std::unique_ptr<CryptoProvider>> CryptoProvider::create(Limits limits, std::stop_token stop) {
    try {
        return from_state(std::make_unique<detail::ProviderState>(limits, stop));
    } catch (const std::bad_alloc &) {
        return Error{ErrorCode::allocation_failed, Operation::initialize};
    } catch (...) {
        return Error{ErrorCode::initialization_failed, Operation::initialize};
    }
}
void CryptoProvider::request_stop() noexcept {
    state_->stop.request_stop();
}
bool CryptoProvider::try_close() noexcept {
    request_stop();
    const std::unique_lock lock(state_->operation, std::try_to_lock);
    return lock.owns_lock();
}
Diagnostic CryptoProvider::diagnostic() const {
    const std::lock_guard lock(state_->accounting->mutex);
    return {state_->accounting->bytes, state_->accounting->handles, state_->cancelled({}), false, false};
}
Result<SecretBuffer> CryptoProvider::random(std::size_t count, std::stop_token token) {
    return state_->run<SecretBuffer>(Operation::random, token, [&] {
        if (count > state_->accounting->limits.random_bytes)
            throw detail::Failure{ErrorCode::invalid_size};
        return state_->generate(count, token);
    });
}
Result<AeadKey> CryptoProvider::generate_aead_key(std::stop_token token) {
    return state_->run<AeadKey>(Operation::generate_key, token,
                                [&] { return AeadKey(state_->generate(aead_key_bytes, token)); });
}
Result<SigningKey> CryptoProvider::generate_signing_key(std::stop_token token) {
    return state_->run<SigningKey>(Operation::generate_key, token,
                                   [&] { return SigningKey(state_->generate(signing_seed_bytes, token)); });
}
Result<AeadKey> CryptoProvider::import_aead_key(std::span<const std::uint8_t> input, std::stop_token token) {
    return state_->run<AeadKey>(Operation::import_key, token, [&] {
        if (input.size() != aead_key_bytes)
            throw detail::Failure{ErrorCode::invalid_size};
        return AeadKey(state_->copy(input));
    });
}
Result<SigningKey> CryptoProvider::import_signing_seed(std::span<const std::uint8_t> input,
                                                       std::stop_token token) {
    return state_->run<SigningKey>(Operation::import_key, token, [&] {
        if (input.size() != signing_seed_bytes)
            throw detail::Failure{ErrorCode::invalid_size};
        return SigningKey(state_->copy(input));
    });
}
Result<PublicKey> CryptoProvider::public_key(const SigningKey &key, std::stop_token token) {
    return state_->run<PublicKey>(Operation::sign, token, [&] {
        if (!key.valid())
            throw detail::Failure{ErrorCode::invalid_key};
        const auto backend = Botan::Ed25519_PrivateKey::from_seed(key.secret_.borrow());
        const auto bytes = backend.raw_public_key_bits();
        if (bytes.size() != public_key_bytes)
            throw detail::Failure{ErrorCode::provider_failure};
        PublicKey output{};
        std::copy(bytes.begin(), bytes.end(), output.begin());
        return output;
    });
}
Result<std::vector<std::uint8_t>> CryptoProvider::encrypt(const AeadKey &key,
                                                          std::span<const std::uint8_t> nonce,
                                                          std::span<const std::uint8_t> plaintext,
                                                          std::span<const std::uint8_t> aad,
                                                          std::stop_token token) {
    return state_->run<std::vector<std::uint8_t>>(Operation::encrypt, token, [&] {
        if (!key.valid())
            throw detail::Failure{ErrorCode::invalid_key};
        if (nonce.size() != aead_nonce_bytes)
            throw detail::Failure{ErrorCode::invalid_size};
        state_->check_message(plaintext.size());
        state_->check_message(aad.size());
        state_->workspace(detail::workspace_overhead + 2 * (plaintext.size() + aad.size() + aead_tag_bytes));
        auto cipher = Botan::AEAD_Mode::create_or_throw("ChaCha20Poly1305", Botan::Cipher_Dir::Encryption);
        cipher->set_key(key.secret_.borrow());
        cipher->set_associated_data(aad);
        cipher->start(nonce);
        Botan::secure_vector<std::uint8_t> work;
        work.reserve(plaintext.size() + aead_tag_bytes);
        work.assign(plaintext.begin(), plaintext.end());
        cipher->finish(work);
        state_->check_cancel(token);
        return std::vector<std::uint8_t>(work.begin(), work.end());
    });
}
Result<SecretBuffer> CryptoProvider::decrypt(const AeadKey &key, std::span<const std::uint8_t> nonce,
                                             std::span<const std::uint8_t> ciphertext,
                                             std::span<const std::uint8_t> aad, std::stop_token token) {
    return state_->run<SecretBuffer>(Operation::decrypt, token, [&] {
        if (!key.valid())
            throw detail::Failure{ErrorCode::invalid_key};
        if (nonce.size() != aead_nonce_bytes || ciphertext.size() < aead_tag_bytes)
            throw detail::Failure{ErrorCode::invalid_size};
        state_->check_message(ciphertext.size() - aead_tag_bytes);
        state_->check_message(aad.size());
        state_->workspace(detail::workspace_overhead + 2 * (ciphertext.size() + aad.size()));
        auto cipher = Botan::AEAD_Mode::create_or_throw("ChaCha20Poly1305", Botan::Cipher_Dir::Decryption);
        cipher->set_key(key.secret_.borrow());
        cipher->set_associated_data(aad);
        cipher->start(nonce);
        Botan::secure_vector<std::uint8_t> work(ciphertext.begin(), ciphertext.end());
        // finish may decrypt before checking the tag. This clearing private buffer
        // is never exposed; only authenticated, uncancelled data is copied out.
        cipher->finish(work);
        state_->check_cancel(token);
        return state_->copy(work);
    });
}
Result<Signature> CryptoProvider::sign_public(const SigningKey &key, std::span<const std::uint8_t> message,
                                              std::stop_token token) {
    return state_->run<Signature>(Operation::sign, token, [&] {
        if (!key.valid())
            throw detail::Failure{ErrorCode::invalid_key};
        state_->check_message(message.size());
        state_->workspace(detail::workspace_overhead + 2 * message.size());
        const auto backend = Botan::Ed25519_PrivateKey::from_seed(key.secret_.borrow());
        Botan::PK_Signer signer(backend, state_->rng, "Pure");
        const auto bytes = signer.sign_message(message, state_->rng);
        if (bytes.size() != signature_bytes)
            throw detail::Failure{ErrorCode::provider_failure};
        Signature output{};
        std::copy(bytes.begin(), bytes.end(), output.begin());
        return output;
    });
}
Result<void> CryptoProvider::verify_public(std::span<const std::uint8_t> key,
                                           std::span<const std::uint8_t> message,
                                           std::span<const std::uint8_t> signature, std::stop_token token) {
    return state_->run<void>(Operation::verify, token, [&] {
        if (key.size() != public_key_bytes || signature.size() != signature_bytes)
            throw detail::Failure{ErrorCode::invalid_size};
        state_->check_message(message.size());
        state_->workspace(detail::workspace_overhead + 2 * message.size());
        const Botan::Ed25519_PublicKey backend(key);
        Botan::PK_Verifier verifier(backend, "Pure");
        if (!verifier.verify_message(message, signature))
            throw detail::Failure{ErrorCode::invalid_signature};
    });
}
Result<Digest> CryptoProvider::sha256(std::span<const std::uint8_t> input, std::stop_token token) {
    return state_->run<Digest>(Operation::hash, token, [&] {
        state_->check_message(input.size());
        auto hash = Botan::HashFunction::create_or_throw("SHA-256");
        hash->update(input);
        Digest output{};
        hash->final(output);
        return output;
    });
}
#ifdef GW_CRYPTO_TESTING
Result<std::unique_ptr<CryptoProvider>> detail::TestAccess::create(std::shared_ptr<TestHooks> hooks,
                                                                   Limits limits, std::stop_token token) {
    try {
        auto state = std::make_unique<ProviderState>(limits, token);
        state->accounting->hooks = std::move(hooks);
        return CryptoProvider::from_state(std::move(state));
    } catch (const std::bad_alloc &) {
        return Error{ErrorCode::allocation_failed, Operation::initialize};
    }
}
#endif
} // namespace ghostwire::crypto
