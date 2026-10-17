#include "ghostwire/composition/identity_service_adapter.hpp"
#include "ghostwire/core/application.hpp"
#include <array>
#include <chrono>
#include <doctest/doctest.h>
#include <filesystem>
#include <sstream>

namespace {
namespace core = ghostwire::core;
namespace identity = ghostwire::identity;
namespace composition = ghostwire::composition;

struct TemporaryRoot {
    std::filesystem::path path =
        std::filesystem::canonical(std::filesystem::temp_directory_path()) /
        ("ghostwire-identity-lifecycle-" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    ~TemporaryRoot() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
    core::Paths paths() const {
        auto resolved = core::resolve_paths(core::Role::cli, path, {});
        REQUIRE(resolved);
        return std::move(resolved).value();
    }
};

struct FailingConsumer final : core::Service {
    composition::IdentityServiceAdapter &adapter;
    core::CryptoService &crypto;
    bool &stopped_with_dependencies;
    FailingConsumer(composition::IdentityServiceAdapter &identity_adapter, core::CryptoService &provider,
                    bool &observed)
        : adapter(identity_adapter), crypto(provider), stopped_with_dependencies(observed) {}
    core::Result<void> start(const core::ApplicationContext &) override {
        if (!adapter.identity())
            return core::Error{core::ErrorCode::internal, "Missing identity dependency."};
        auto created = adapter.identity()->create_profile("12345", "Synthetic private label",
                                                          identity::VerificationTime{100, true});
        if (!created)
            return core::Error{core::ErrorCode::internal, "Profile setup failed."};
        return core::Error{core::ErrorCode::internal, "Deliberate consumer startup failure."};
    }
    core::Result<void> stop(const core::ShutdownContext &) override {
        stopped_with_dependencies = adapter.identity() && crypto.provider() &&
                                    crypto.provider()->diagnostic().live_secret_handles == 2;
        return {};
    }
};
} // namespace

TEST_CASE("identity lifecycle explicitly composes dependencies and clears keys before provider teardown") {
    TemporaryRoot root;
    std::ostringstream logs;
    core::Logger logger(logs);
    core::Application app(core::Role::cli, {}, root.paths(), logger);
    auto crypto_service = std::make_unique<core::CryptoService>();
    auto *crypto = crypto_service.get();
    auto identity_service = std::make_unique<composition::IdentityServiceAdapter>(*crypto);
    auto *adapter = identity_service.get();
    CHECK(adapter->identity() == nullptr);
    constexpr std::array dependencies{core::cryptographic_provider_service};
    // Reverse registration deliberately exercises dependency sorting.
    REQUIRE(app.add_service(composition::identity_service, std::move(identity_service), dependencies));
    REQUIRE(app.add_service(core::cryptographic_provider_service, std::move(crypto_service)));
    REQUIRE(app.start());
    REQUIRE(adapter->identity() != nullptr);
    REQUIRE(crypto->provider() != nullptr);
    CHECK(adapter->identity()->diagnostic().profiles == 0);
    CHECK(crypto->provider()->diagnostic().live_secret_handles == 0);
    CHECK(app.context().admission().snapshot().active == 1);
    CHECK(app.context().admission().snapshot().reserved_bytes == 2 * 1024 * 1024 + 128 * 32);
    auto profile = adapter->identity()->create_profile("54321", "PRIVATE_MARKER_LIFECYCLE",
                                                       identity::VerificationTime{100, true});
    REQUIRE(profile);
    CHECK(crypto->provider()->diagnostic().live_secret_handles == 2);
    app.request_stop();
    auto cancelled = adapter->identity()->create_profile("1", "No publication", {100, true});
    REQUIRE_FALSE(cancelled);
    CHECK(cancelled.error().code == identity::ErrorCode::cancelled);
    REQUIRE(app.shutdown());
    CHECK(adapter->identity() == nullptr);
    CHECK(crypto->provider() == nullptr);
    CHECK(app.context().admission().snapshot().reserved_bytes == 0);
    CHECK(app.context().admission().snapshot().active == 0);
    CHECK(app.shutdown());
    for (const auto &entry : std::filesystem::recursive_directory_iterator(root.path))
        CHECK(entry.is_directory());
    CHECK(logs.str().find("PRIVATE_MARKER_LIFECYCLE") == std::string::npos);
    CHECK(logs.str().find("54321") == std::string::npos);
}

TEST_CASE("identity lifecycle rolls back initialized keys and retains dependencies during consumer cleanup") {
    TemporaryRoot root;
    std::ostringstream logs;
    core::Logger logger(logs, core::LogLevel::off);
    core::Application app(core::Role::cli, {}, root.paths(), logger);
    auto crypto_service = std::make_unique<core::CryptoService>();
    auto *crypto = crypto_service.get();
    auto identity_service = std::make_unique<composition::IdentityServiceAdapter>(*crypto);
    auto *adapter = identity_service.get();
    bool stopped_with_dependencies = false;
    constexpr std::array crypto_dependency{core::cryptographic_provider_service};
    constexpr std::array identity_dependency{composition::identity_service};
    REQUIRE(app.add_service({4},
                            std::make_unique<FailingConsumer>(*adapter, *crypto, stopped_with_dependencies),
                            identity_dependency));
    REQUIRE(app.add_service(composition::identity_service, std::move(identity_service), crypto_dependency));
    REQUIRE(app.add_service(core::cryptographic_provider_service, std::move(crypto_service)));
    CHECK_FALSE(app.start());
    CHECK(stopped_with_dependencies);
    CHECK(adapter->identity() == nullptr);
    CHECK(crypto->provider() == nullptr);
    CHECK(app.context().admission().snapshot().active == 0);
    CHECK(app.context().admission().snapshot().reserved_bytes == 0);
    CHECK(app.shutdown());
    CHECK(logs.str().empty());
}

TEST_CASE("identity lifecycle rejects missing dependency and invalid provider without creating profiles") {
    TemporaryRoot root;
    std::ostringstream logs;
    core::Logger logger(logs, core::LogLevel::off);
    SUBCASE("Missing graph dependency") {
        core::Application app(core::Role::cli, {}, root.paths(), logger);
        core::CryptoService unregistered;
        constexpr std::array dependency{core::cryptographic_provider_service};
        REQUIRE(app.add_service(composition::identity_service,
                                std::make_unique<composition::IdentityServiceAdapter>(unregistered),
                                dependency));
        auto started = app.start();
        REQUIRE_FALSE(started);
        CHECK(started.error().reason == core::ErrorReason::missing_dependency);
        CHECK(unregistered.provider() == nullptr);
    }
    SUBCASE("Provider initialization fails") {
        core::Application app(core::Role::cli, {}, root.paths(), logger);
        ghostwire::crypto::Limits limits;
        limits.live_secret_handles = 0;
        auto crypto_service = std::make_unique<core::CryptoService>(limits);
        auto *crypto = crypto_service.get();
        auto identity_service = std::make_unique<composition::IdentityServiceAdapter>(*crypto);
        auto *adapter = identity_service.get();
        constexpr std::array dependency{core::cryptographic_provider_service};
        REQUIRE(app.add_service(core::cryptographic_provider_service, std::move(crypto_service)));
        REQUIRE(app.add_service(composition::identity_service, std::move(identity_service), dependency));
        CHECK_FALSE(app.start());
        CHECK(adapter->identity() == nullptr);
        CHECK(crypto->provider() == nullptr);
        CHECK(app.context().admission().snapshot().reserved_bytes == 0);
    }
}

TEST_CASE("identity lifecycle admission rejects insufficient declared capacity and releases failed startup") {
    TemporaryRoot root;
    std::ostringstream logs;
    core::Logger logger(logs, core::LogLevel::off);
    core::ApplicationOptions options;
    options.resources.memory_budget_bytes = 1024;
    core::Application app(core::Role::cli, {}, root.paths(), logger, options);
    auto crypto_service = std::make_unique<core::CryptoService>();
    auto *crypto = crypto_service.get();
    auto identity_service = std::make_unique<composition::IdentityServiceAdapter>(*crypto);
    auto *adapter = identity_service.get();
    constexpr std::array dependency{core::cryptographic_provider_service};
    REQUIRE(app.add_service(core::cryptographic_provider_service, std::move(crypto_service)));
    REQUIRE(app.add_service(composition::identity_service, std::move(identity_service), dependency));
    auto started = app.start();
    REQUIRE_FALSE(started);
    // The existing manager intentionally normalizes service failures, retaining
    // only the developer-assigned service ID in its public startup result.
    CHECK(started.error().reason == core::ErrorReason::service_start_failed);
    CHECK(started.error().subject == composition::identity_service.value);
    CHECK(adapter->identity() == nullptr);
    CHECK(crypto->provider() == nullptr);
    CHECK(app.context().admission().snapshot().reserved_bytes == 0);
    CHECK(app.context().admission().snapshot().active == 0);
    CHECK(app.context().admission().snapshot().rejected == 1);
    CHECK(app.shutdown());

    // At the adapter boundary the actual admission cause remains structured.
    // Exercise that hook directly with its dependency already running, without
    // weakening the manager's established error-redaction contract.
    core::Application direct(core::Role::cli, {}, root.paths(), logger, options);
    auto direct_crypto_service = std::make_unique<core::CryptoService>();
    auto *direct_crypto = direct_crypto_service.get();
    REQUIRE(direct.add_service(core::cryptographic_provider_service, std::move(direct_crypto_service)));
    REQUIRE(direct.start());
    composition::IdentityServiceAdapter direct_adapter(*direct_crypto);
    auto rejected = direct_adapter.start(direct.context());
    REQUIRE_FALSE(rejected);
    CHECK(rejected.error().reason == core::ErrorReason::resource_exhausted);
    CHECK(direct_adapter.identity() == nullptr);
    CHECK(direct.context().admission().snapshot().reserved_bytes == 0);
    auto deadline = core::Deadline::after(direct.context().clock(), std::chrono::milliseconds(0));
    REQUIRE(deadline);
    CHECK(direct_adapter.stop({direct.context().clock(), deadline.value(), direct.stop_token()}));
    CHECK(direct.shutdown());
}

TEST_CASE("identity lifecycle empty cleanup is terminal and repeated cleanup is safe") {
    TemporaryRoot root;
    std::ostringstream logs;
    core::Logger logger(logs, core::LogLevel::off);
    core::Application app(core::Role::cli, {}, root.paths(), logger);
    core::CryptoService crypto;
    composition::IdentityServiceAdapter adapter(crypto);
    auto deadline = core::Deadline::after(app.context().clock(), std::chrono::milliseconds(0));
    REQUIRE(deadline);
    const core::ShutdownContext shutdown{app.context().clock(), deadline.value(), app.stop_token()};
    CHECK(adapter.stop(shutdown));
    CHECK(adapter.stop(shutdown));
    auto started = adapter.start(app.context());
    REQUIRE_FALSE(started);
    CHECK(started.error().reason == core::ErrorReason::invalid_state);
    CHECK(adapter.identity() == nullptr);
}
