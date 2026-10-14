#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ghostwire/core/application.hpp"
#include "ghostwire/core/crypto_service.hpp"
#include "ghostwire/crypto/crypto_provider.hpp"
#include <algorithm>
#include <barrier>
#include <chrono>
#include <doctest/doctest.h>
#include <filesystem>
#include <limits>
#include <sstream>
#include <thread>
#include <type_traits>

using namespace ghostwire::crypto;
namespace {
std::vector<std::uint8_t> hex(std::string_view text) {
    REQUIRE(text.size() % 2 == 0);
    auto digit = [](char value) -> unsigned {
        if (value >= '0' && value <= '9')
            return static_cast<unsigned>(value - '0');
        if (value >= 'a' && value <= 'f')
            return static_cast<unsigned>(value - 'a' + 10);
        if (value >= 'A' && value <= 'F')
            return static_cast<unsigned>(value - 'A' + 10);
        FAIL("Invalid test vector hex");
        return 0;
    };
    std::vector<std::uint8_t> output;
    for (std::size_t i = 0; i < text.size(); i += 2)
        output.push_back(static_cast<std::uint8_t>(16 * digit(text[i]) + digit(text[i + 1])));
    return output;
}
bool equal(std::span<const std::uint8_t> a, std::span<const std::uint8_t> b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end());
}
std::unique_ptr<CryptoProvider> provider(Limits limits = {}) {
    auto result = CryptoProvider::create(limits);
    REQUIRE(result);
    return std::move(result).value();
}
template <typename T> void error(const Result<T> &result, ErrorCode expected) {
    REQUIRE_FALSE(result);
    CHECK(result.error().code == expected);
    CHECK_FALSE(describe(result.error().code).empty());
}
struct AeadVector {
    const char *key;
    const char *nonce;
    const char *aad;
    const char *input;
    const char *output;
};
// RFC 8439 2.8.2 (also upstream RFC 7539 vector), and Botan's published
// OpenSSL-generated exact-block regression vectors. Provenance: fixtures/crypto/README.md.
constexpr AeadVector vectors[] = {
    {"808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f", "070000004041424344454647",
     "50515253c0c1c2c3c4c5c6c7",
     "4c616469657320616e642047656e746c656d656e206f662074686520636c617373206f66202739393a204966204920636f756c6"
     "4206f6666657220796f75206f6e6c79206f6e652074697020666f7220746865206675747572652c2073756e73637265656e2077"
     "6f756c642062652069742e",
     "d31a8d34648e60db7b86afbc53ef7ec2a4aded51296e08fea9e2b5a736ee62d63dbea45e8ca9671282fafb69da92728b1a71de0"
     "a9e060b2905d6a5b67ecd3b3692ddbd7f2d778b8c9803aee328091b58fab324e4fad675945585808b4831d7bc3ff4def08e4b7a"
     "9de576d26586cec64b61161ae10b594f09e26a7e902ecbd0600691"},
    {"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "BBBBBBBBBBBBBBBBBBBBBBBB",
     "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC", "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD",
     "A0C9391216A037370BDFF40626C5DD137E93A01836AF9B8A5C1F929EF18C9350"},
    {"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "BBBBBBBBBBBBBBBBBBBBBBBB",
     "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC", "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD",
     "A0C9391216A037370BDFF40626C5DD13422D6D1564BCE2074D98279498CE4C86"},
    {"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "BBBBBBBBBBBBBBBBBBBBBBBB",
     "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC", "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
     "A0C9391216A037370BDFF40626C5DD13D45447FBEBA3C985BF65FBCBE51663F9214F9C6757F9FC0CFF3135E68DC7251F"}};
} // namespace

static_assert(!std::is_copy_constructible_v<SecretBuffer> && !std::is_copy_assignable_v<SecretBuffer>);
static_assert(!std::is_copy_constructible_v<AeadKey> && !std::is_copy_assignable_v<AeadKey>);
static_assert(!std::is_copy_constructible_v<SigningKey> && !std::is_copy_assignable_v<SigningKey>);
static_assert(std::is_nothrow_move_constructible_v<SecretBuffer>);
static_assert(std::is_nothrow_move_assignable_v<SigningKey>);
static_assert(!std::is_convertible_v<AeadKey, SigningKey>);
static_assert(!std::is_convertible_v<SecretBuffer, std::string>);
static_assert(!std::is_move_constructible_v<CryptoProvider>);

TEST_CASE("crypto provider version capabilities and owned initialization") {
    auto crypto = provider();
    CHECK(CryptoProvider::backend_version() == "Botan 3.13.0");
    CHECK(crypto->diagnostic().live_secret_bytes == 0);
    CHECK_FALSE(crypto->diagnostic().all_secret_memory_locked);
    CHECK_FALSE(crypto->diagnostic().persistence_available);
    CHECK_FALSE(crypto->diagnostic().stopped);
}
TEST_CASE("crypto rejects invalid limits before cryptographic work") {
    Limits v;
    v.live_secret_bytes = 0;
    error(CryptoProvider::create(v), ErrorCode::invalid_limits);
    v = {};
    v.live_secret_handles = 0;
    error(CryptoProvider::create(v), ErrorCode::invalid_limits);
    v = {};
    v.message_bytes = maximum_input_bytes + 1;
    error(CryptoProvider::create(v), ErrorCode::invalid_limits);
    v = {};
    v.random_bytes = 0;
    error(CryptoProvider::create(v), ErrorCode::invalid_limits);
    v = {};
    v.random_bytes = maximum_input_bytes + 1;
    error(CryptoProvider::create(v), ErrorCode::invalid_limits);
    v = {};
    v.workspace_bytes = 1;
    error(CryptoProvider::create(v), ErrorCode::invalid_limits);
    v = {};
    v.live_secret_bytes = std::numeric_limits<std::size_t>::max();
    error(CryptoProvider::create(v), ErrorCode::invalid_limits);
    v = {};
    v.live_secret_handles = 4097;
    error(CryptoProvider::create(v), ErrorCode::invalid_limits);
}
TEST_CASE("crypto OS random returns owned bounded bytes and supports zero length") {
    Limits limits;
    limits.random_bytes = 4097;
    auto crypto = provider(limits);
    auto zero = crypto->random(0);
    REQUIRE(zero);
    CHECK(zero.value().size() == 0);
    auto a = crypto->random(32), b = crypto->random(32);
    REQUIRE(a);
    REQUIRE(b);
    CHECK(a.value().size() == 32);
    CHECK_FALSE(equal(a.value().borrow(), b.value().borrow())); // Sanity, not an entropy proof.
    auto boundary = crypto->random(4097);
    REQUIRE(boundary);
    CHECK(boundary.value().size() == 4097);
    error(crypto->random(4098), ErrorCode::invalid_size);
    error(crypto->random(std::numeric_limits<std::size_t>::max()), ErrorCode::invalid_size);
}
TEST_CASE("crypto RFC8439 and upstream OpenSSL AEAD known answers") {
    auto crypto = provider();
    for (const auto &vector : vectors) {
        const auto nonce = hex(vector.nonce), aad = hex(vector.aad), input = hex(vector.input),
                   expected = hex(vector.output);
        auto key = crypto->import_aead_key(hex(vector.key));
        REQUIRE(key);
        auto encrypted = crypto->encrypt(key.value(), nonce, input, aad);
        REQUIRE(encrypted);
        CHECK(equal(encrypted.value(), expected));
        auto decrypted = crypto->decrypt(key.value(), nonce, expected, aad);
        REQUIRE(decrypted);
        CHECK(equal(decrypted.value().borrow(), input));
    }
}
TEST_CASE("crypto rejects every modified ciphertext byte without releasing plaintext") {
    auto crypto = provider();
    const auto &v = vectors[0];
    auto key = crypto->import_aead_key(hex(v.key));
    REQUIRE(key);
    const auto nonce = hex(v.nonce), aad = hex(v.aad), ciphertext = hex(v.output);
    for (std::size_t i = 0; i < ciphertext.size(); ++i) {
        auto damaged = ciphertext;
        damaged[i] ^= 1;
        auto rejected = crypto->decrypt(key.value(), nonce, damaged, aad);
        error(rejected, ErrorCode::authentication_failed);
        CHECK(rejected.error().operation == Operation::decrypt);
        CHECK(crypto->diagnostic().live_secret_bytes == 32);
        CHECK(crypto->diagnostic().live_secret_handles == 1);
    }
}
TEST_CASE("crypto authenticates AAD nonce and key and rejects truncation or extension") {
    auto crypto = provider();
    const auto &v = vectors[0];
    auto key = crypto->import_aead_key(hex(v.key));
    REQUIRE(key);
    auto nonce = hex(v.nonce), aad = hex(v.aad), ciphertext = hex(v.output);
    auto wrong_key = crypto->generate_aead_key();
    REQUIRE(wrong_key);
    error(crypto->decrypt(wrong_key.value(), nonce, ciphertext, aad), ErrorCode::authentication_failed);
    aad[0] ^= 1;
    error(crypto->decrypt(key.value(), nonce, ciphertext, aad), ErrorCode::authentication_failed);
    aad[0] ^= 1;
    nonce[0] ^= 1;
    error(crypto->decrypt(key.value(), nonce, ciphertext, aad), ErrorCode::authentication_failed);
    nonce[0] ^= 1;
    error(crypto->decrypt(key.value(), nonce, std::span(ciphertext).first(ciphertext.size() - 1), aad),
          ErrorCode::authentication_failed);
    ciphertext.push_back(0);
    error(crypto->decrypt(key.value(), nonce, ciphertext, aad), ErrorCode::authentication_failed);
}
TEST_CASE("crypto AEAD zero block and policy-boundary inputs roundtrip") {
    Limits limits;
    limits.message_bytes = 4096;
    auto crypto = provider(limits);
    auto key = crypto->generate_aead_key();
    REQUIRE(key);
    std::array<std::uint8_t, 12> nonce{};
    for (const std::size_t size : {0U, 1U, 15U, 16U, 17U, 63U, 64U, 65U, 4096U}) {
        ++nonce.back(); // Unique per encryption under this volatile key.
        const std::vector<std::uint8_t> input(size, 0xa5), aad(size, 0x4c);
        auto ciphertext = crypto->encrypt(key.value(), nonce, input, aad);
        REQUIRE(ciphertext);
        CHECK(ciphertext.value().size() == size + aead_tag_bytes);
        auto plaintext = crypto->decrypt(key.value(), nonce, ciphertext.value(), aad);
        REQUIRE(plaintext);
        CHECK(equal(plaintext.value().borrow(), input));
    }
}
TEST_CASE("crypto key nonce and AEAD size validation is strict") {
    Limits limits;
    limits.message_bytes = 32;
    auto crypto = provider(limits);
    std::array<std::uint8_t, 64> bytes{};
    for (const std::size_t size : {0U, 1U, 31U, 33U, 64U}) {
        error(crypto->import_aead_key(std::span(bytes).first(size)), ErrorCode::invalid_size);
        error(crypto->import_signing_seed(std::span(bytes).first(size)), ErrorCode::invalid_size);
    }
    auto key = crypto->generate_aead_key();
    REQUIRE(key);
    for (const std::size_t size : {0U, 8U, 11U, 13U, 24U}) {
        error(crypto->encrypt(key.value(), std::span(bytes).first(size), {}), ErrorCode::invalid_size);
        error(crypto->decrypt(key.value(), std::span(bytes).first(size), std::span(bytes).first(16)),
              ErrorCode::invalid_size);
    }
    const auto nonce = std::span(bytes).first(12);
    error(crypto->encrypt(key.value(), nonce, std::span(bytes).first(33)), ErrorCode::invalid_size);
    error(crypto->encrypt(key.value(), nonce, {}, std::span(bytes).first(33)), ErrorCode::invalid_size);
    error(crypto->decrypt(key.value(), nonce, std::span(bytes).first(15)), ErrorCode::invalid_size);
    error(crypto->decrypt(key.value(), nonce, std::span(bytes).first(49)), ErrorCode::invalid_size);
    error(crypto->decrypt(key.value(), nonce, std::span(bytes).first(16), std::span(bytes).first(33)),
          ErrorCode::invalid_size);
    error(crypto->encrypt(AeadKey{}, nonce, {}), ErrorCode::invalid_key);
}
TEST_CASE("crypto RFC8032 pure Ed25519 public key and signature known answers") {
    struct Vector {
        const char *seed;
        const char *public_key;
        const char *message;
        const char *signature;
    };
    constexpr Vector answers[] = {{"9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60",
                                   "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a", "",
                                   "e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb882159"
                                   "0a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b"},
                                  {"4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb",
                                   "3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c", "72",
                                   "92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da085ac1e43"
                                   "e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00"},
                                  {"c5aa8df43f9f837bedb7442f31dcb7b166d38535076f094b85ce3a2e0b4458f7",
                                   "fc51cd8e6218a1a38da47ed00230f0580816ed13ba3303ac5deb911548908025", "af82",
                                   "6291d657deec24024827e69c3abe01a30ce548a284743a445e3680d7db5ac3ac18ff9b538"
                                   "d16f290ae67f760984dc6594a7c15e9716ed28dc027beceea1ec40a"}};
    auto crypto = provider();
    for (const auto &v : answers) {
        auto key = crypto->import_signing_seed(hex(v.seed));
        REQUIRE(key);
        auto pub = crypto->public_key(key.value());
        REQUIRE(pub);
        CHECK(equal(pub.value(), hex(v.public_key)));
        auto signature = crypto->sign_public(key.value(), hex(v.message));
        REQUIRE(signature);
        CHECK(equal(signature.value(), hex(v.signature)));
        CHECK(crypto->verify_public(hex(v.public_key), hex(v.message), hex(v.signature)));
    }
}
TEST_CASE("crypto Ed25519 rejects altered signature message key and malformed lengths") {
    auto crypto = provider();
    auto key = crypto->generate_signing_key();
    REQUIRE(key);
    auto pub = crypto->public_key(key.value());
    REQUIRE(pub);
    const auto message = hex("0010203040");
    auto signature = crypto->sign_public(key.value(), message);
    REQUIRE(signature);
    for (std::size_t i = 0; i < signature_bytes; ++i) {
        auto damaged = signature.value();
        damaged[i] ^= 1;
        error(crypto->verify_public(pub.value(), message, damaged), ErrorCode::invalid_signature);
    }
    error(crypto->verify_public(pub.value(), {}, signature.value()), ErrorCode::invalid_signature);
    auto other = crypto->generate_signing_key();
    REQUIRE(other);
    auto other_pub = crypto->public_key(other.value());
    REQUIRE(other_pub);
    error(crypto->verify_public(other_pub.value(), message, signature.value()), ErrorCode::invalid_signature);
    error(crypto->verify_public(std::span(pub.value()).first(31), message, signature.value()),
          ErrorCode::invalid_size);
    error(crypto->verify_public(pub.value(), message, std::span(signature.value()).first(63)),
          ErrorCode::invalid_size);
    error(crypto->sign_public(SigningKey{}, {}), ErrorCode::invalid_key);
    error(crypto->public_key(SigningKey{}), ErrorCode::invalid_key);
}
TEST_CASE("crypto rejects upstream noncanonical and small-order Ed25519 forgeries") {
    auto crypto = provider();
    const auto message = hex("41");
    const auto forged = hex("01000000000000000000000000000000000000000000000000000000000000000000000000000000"
                            "000000000000000000000000000000000000000000000000");
    error(crypto->verify_public(hex("0100000000000000000000000000000000000000000000000000000000000000"),
                                message, forged),
          ErrorCode::invalid_signature);
    error(crypto->verify_public(hex("0100000000000000000000000000000000000000000000000000000000000080"),
                                message, forged),
          ErrorCode::invalid_signature);
}
TEST_CASE("crypto SHA256 published empty abc and multiblock known answers") {
    auto crypto = provider();
    const std::pair<const char *, const char *> answers[] = {
        {"", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"},
        {"616263", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"},
        {"6162636462636465636465666465666765666768666768696768696a68696a6b696a6b6c6a6b6c6d6b6c6d6e6c6d6e6f6d6"
         "e6f706e6f7071",
         "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"}};
    for (const auto &[input, expected] : answers) {
        auto digest = crypto->sha256(hex(input));
        REQUIRE(digest);
        CHECK(equal(digest.value(), hex(expected)));
    }
}
TEST_CASE("crypto signing verification and hashing enforce message bounds") {
    Limits limits;
    limits.message_bytes = 16;
    auto crypto = provider(limits);
    auto key = crypto->generate_signing_key();
    REQUIRE(key);
    auto pub = crypto->public_key(key.value());
    REQUIRE(pub);
    std::vector<std::uint8_t> message(16, 0);
    auto sig = crypto->sign_public(key.value(), message);
    REQUIRE(sig);
    CHECK(crypto->verify_public(pub.value(), message, sig.value()));
    CHECK(crypto->sha256(message));
    message.push_back(0);
    error(crypto->sign_public(key.value(), message), ErrorCode::invalid_size);
    error(crypto->verify_public(pub.value(), message, sig.value()), ErrorCode::invalid_size);
    error(crypto->sha256(message), ErrorCode::invalid_size);
}
TEST_CASE("crypto retained secrets enforce bytes and handle budgets with move-only release") {
    Limits limits;
    limits.live_secret_bytes = 64;
    limits.live_secret_handles = 2;
    auto crypto = provider(limits);
    auto first = crypto->random(32), second = crypto->random(32);
    REQUIRE(first);
    REQUIRE(second);
    error(crypto->random(1), ErrorCode::resource_exhausted);
    SecretBuffer held = std::move(first).value();
    CHECK(first.value().size() == 0);
    CHECK(crypto->diagnostic().live_secret_bytes == 64);
    held = std::move(second).value();
    CHECK(second.value().size() == 0);
    CHECK(crypto->diagnostic().live_secret_bytes == 32);
    CHECK(crypto->diagnostic().live_secret_handles == 1);
    held.reset();
    held.reset();
    CHECK(crypto->diagnostic().live_secret_bytes == 0);
    CHECK(crypto->random(64));
    limits.live_secret_bytes = 1024;
    limits.live_secret_handles = 1;
    auto one = provider(limits);
    auto tiny = one->random(1);
    REQUIRE(tiny);
    error(one->generate_aead_key(), ErrorCode::resource_exhausted);
    CHECK(one->random(0)); // Empty results hold no allocation.
}
TEST_CASE("crypto workspace admission fails before bulk operations") {
    Limits limits;
    limits.workspace_bytes = 64 * 1024;
    auto crypto = provider(limits);
    auto key = crypto->generate_aead_key();
    REQUIRE(key);
    auto signing = crypto->generate_signing_key();
    REQUIRE(signing);
    const auto pub = crypto->public_key(signing.value());
    REQUIRE(pub);
    const std::array<std::uint8_t, 12> nonce{};
    const std::array<std::uint8_t, 16> ciphertext{};
    error(crypto->encrypt(key.value(), nonce, {}), ErrorCode::resource_exhausted);
    error(crypto->decrypt(key.value(), nonce, ciphertext), ErrorCode::resource_exhausted);
    error(crypto->sign_public(signing.value(), hex("00")), ErrorCode::resource_exhausted);
    error(crypto->verify_public(pub.value(), hex("00"), Signature{}), ErrorCode::resource_exhausted);
    CHECK(crypto->diagnostic().live_secret_bytes == 64);
}
TEST_CASE("crypto failed output allocation releases no plaintext or reservation") {
    Limits limits;
    limits.live_secret_bytes = 32;
    auto crypto = provider(limits);
    const auto &v = vectors[0];
    auto key = crypto->import_aead_key(hex(v.key));
    REQUIRE(key);
    error(crypto->decrypt(key.value(), hex(v.nonce), hex(v.output), hex(v.aad)),
          ErrorCode::resource_exhausted);
    CHECK(crypto->diagnostic().live_secret_bytes == 32);
    CHECK(crypto->diagnostic().live_secret_handles == 1);
}
TEST_CASE("crypto keys and buffers outlive provider and moved-from keys are rejected") {
    SecretBuffer buffer;
    AeadKey key;
    SigningKey signing;
    {
        auto crypto = provider();
        auto generated = crypto->generate_aead_key();
        REQUIRE(generated);
        key = std::move(generated).value();
        auto seed = crypto->generate_signing_key();
        REQUIRE(seed);
        signing = std::move(seed).value();
        CHECK_FALSE(generated.value().valid());
        CHECK_FALSE(seed.value().valid());
        auto random = crypto->random(32);
        REQUIRE(random);
        buffer = std::move(random).value();
    }
    CHECK(buffer.size() == 32);
    CHECK(key.valid());
    CHECK(signing.valid());
    auto another = provider(); // Primitive handles are independent of provider lifetime.
    CHECK(another->public_key(signing));
    const std::array<std::uint8_t, 12> nonce{};
    CHECK(another->encrypt(key, nonce, {}));
    AeadKey moved = std::move(key);
    error(another->encrypt(key, nonce, {}), ErrorCode::invalid_key);
    moved.reset();
    signing.reset();
    buffer.reset();
}
TEST_CASE("crypto per-call application and owned cancellation remain distinct") {
    std::stop_source application;
    auto created = CryptoProvider::create({}, application.get_token());
    REQUIRE(created);
    auto crypto = std::move(created).value();
    auto key = crypto->generate_aead_key();
    REQUIRE(key);
    auto signing = crypto->generate_signing_key();
    REQUIRE(signing);
    auto pub = crypto->public_key(signing.value());
    REQUIRE(pub);
    std::stop_source call;
    call.request_stop();
    auto token = call.get_token();
    error(crypto->random(32, token), ErrorCode::cancelled);
    error(crypto->generate_aead_key(token), ErrorCode::cancelled);
    error(crypto->generate_signing_key(token), ErrorCode::cancelled);
    error(crypto->import_aead_key({}, token), ErrorCode::cancelled);
    error(crypto->import_signing_seed({}, token), ErrorCode::cancelled);
    error(crypto->public_key(signing.value(), token), ErrorCode::cancelled);
    error(crypto->encrypt(key.value(), {}, {}, {}, token), ErrorCode::cancelled);
    error(crypto->decrypt(key.value(), {}, {}, {}, token), ErrorCode::cancelled);
    error(crypto->sign_public(signing.value(), {}, token), ErrorCode::cancelled);
    error(crypto->verify_public(pub.value(), {}, Signature{}, token), ErrorCode::cancelled);
    error(crypto->sha256({}, token), ErrorCode::cancelled);
    CHECK(crypto->random(1));
    application.request_stop();
    error(crypto->random(1), ErrorCode::cancelled);
    error(CryptoProvider::create({}, application.get_token()), ErrorCode::cancelled);
    CHECK(crypto->try_close());
    CHECK(crypto->try_close());
    CHECK(crypto->diagnostic().stopped);
    auto separate = provider();
    separate->request_stop();
    error(separate->sha256({}), ErrorCode::cancelled);
}
TEST_CASE("crypto shared provider calls either complete correctly or report bounded backpressure") {
    auto crypto = provider();
    std::barrier start(4);
    std::array<bool, 4> correct{};
    std::vector<std::jthread> threads;
    for (std::size_t i = 0; i < correct.size(); ++i)
        threads.emplace_back([&, i] {
            start.arrive_and_wait();
            auto result = crypto->sha256({});
            correct[i] = result ? result.value()[0] == 0xe3 : result.error().code == ErrorCode::busy;
        });
    threads.clear();
    CHECK(std::all_of(correct.begin(), correct.end(), [](bool b) { return b; }));
    CHECK(crypto->diagnostic().live_secret_bytes == 0);
}

TEST_CASE("crypto real lifecycle service starts stops and propagates application cancellation") {
    namespace core = ghostwire::core;
    std::ostringstream logs;
    core::Logger logger(logs, core::LogLevel::off);
    const auto path = std::filesystem::temp_directory_path() /
                      ("ghostwire-crypto-lifecycle-" +
                       std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    struct Cleanup {
        std::filesystem::path path;
        ~Cleanup() {
            std::error_code ec;
            std::filesystem::remove_all(path, ec);
        }
    } cleanup{path};
    auto paths = core::resolve_paths(core::Role::cli, path, {});
    REQUIRE(paths);
    core::Application app(core::Role::cli, {}, std::move(paths).value(), logger);
    auto service = std::make_unique<core::CryptoService>();
    auto *borrowed = service.get();
    CHECK(borrowed->provider() == nullptr);
    REQUIRE(app.add_service(core::cryptographic_provider_service, std::move(service)));
    REQUIRE(app.start());
    REQUIRE(borrowed->provider() != nullptr);
    auto key = borrowed->provider()->generate_signing_key();
    REQUIRE(key);
    auto pub = borrowed->provider()->public_key(key.value());
    REQUIRE(pub);
    auto signature = borrowed->provider()->sign_public(key.value(), {});
    REQUIRE(signature);
    CHECK(borrowed->provider()->verify_public(pub.value(), {}, signature.value()));
    app.request_stop();
    error(borrowed->provider()->random(32), ErrorCode::cancelled);
    CHECK(app.shutdown());
    CHECK(borrowed->provider() == nullptr);
    CHECK(app.shutdown());
    CHECK(key.value().valid()); // Safe volatile ownership, no persistence on shutdown.
    for (const auto &entry : std::filesystem::recursive_directory_iterator(path))
        CHECK(entry.is_directory());
    CHECK(logs.str().empty());
}
