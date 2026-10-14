#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ghostwire/core/bootstrap.hpp"
#include "ghostwire/crypto/crypto_provider.hpp"
#include "ghostwire/platform/filesystem.hpp"
#include <atomic>
#include <cerrno>
#include <chrono>
#include <doctest/doctest.h>
#include <filesystem>
#include <sstream>
#include <sys/random.h>

// Linux test-executable-only linker wrapping. This calls the actual pinned
// Botan System_RNG error path through the production GhostWire library.
namespace {
std::atomic<bool> deny_entropy{false};
}
extern "C" ssize_t __real_getrandom(void *, size_t, unsigned int);
extern "C" ssize_t __wrap_getrandom(void *data, size_t length, unsigned int flags) {
    if (deny_entropy.load()) {
        errno = EIO;
        return -1;
    }
    return __real_getrandom(data, length, flags);
}
TEST_CASE("production Botan OS entropy failure is recoverable and never downgrades") {
    using namespace ghostwire::crypto;
    deny_entropy = true;
    auto failed = CryptoProvider::create();
    REQUIRE_FALSE(failed);
    CHECK(failed.error().code == ErrorCode::entropy_unavailable);
    deny_entropy = false;
    auto made = CryptoProvider::create();
    REQUIRE(made);
    auto crypto = std::move(made).value();
    deny_entropy = true;
    auto random = crypto->random(32);
    REQUIRE_FALSE(random);
    CHECK(random.error().code == ErrorCode::entropy_unavailable);
    auto key = crypto->generate_aead_key();
    REQUIRE_FALSE(key);
    CHECK(key.error().code == ErrorCode::entropy_unavailable);
    auto signing = crypto->generate_signing_key();
    REQUIRE_FALSE(signing);
    CHECK(signing.error().code == ErrorCode::entropy_unavailable);
    CHECK(crypto->diagnostic().live_secret_bytes == 0);
    deny_entropy = false;
    CHECK(crypto->generate_aead_key());
}
TEST_CASE("production bootstrap entropy failure exits 70 without successful startup") {
    namespace fs = std::filesystem;
    using namespace ghostwire;
    const auto path = fs::temp_directory_path() /
                      ("ghostwire-entropy-failure-" +
                       std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    struct Cleanup {
        fs::path path;
        ~Cleanup() {
            std::error_code ec;
            fs::remove_all(path, ec);
            deny_entropy = false;
        }
    } cleanup{path};
    const auto root = platform::path_to_utf8(path);
    const std::array<std::string_view, 2> args{"--data-root", root};
    std::ostringstream output, errors;
    deny_entropy = true;
    CHECK(core::run_cli(core::Role::cli, args, output, errors) == 70);
    CHECK(output.str().empty());
    CHECK(errors.str().find("startup") == std::string::npos);
    CHECK(errors.str().find(root) == std::string::npos);
    CHECK(errors.str().find("getrandom") == std::string::npos);
    for (const auto &entry : fs::recursive_directory_iterator(path))
        CHECK(entry.is_directory());
}
