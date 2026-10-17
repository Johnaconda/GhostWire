#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ghostwire/identity/identity_service.hpp"
#include "testing.hpp"
#include <algorithm>
#include <atomic>
#include <doctest/doctest.h>
#include <latch>
#include <optional>
#include <stdexcept>
#include <thread>

namespace gi = ghostwire::identity;
namespace gc = ghostwire::crypto;
using gc::detail::TestAccess;
using gc::detail::TestHooks;
namespace {
template <typename T, typename E> T take(ghostwire::core::Result<T, E> result) {
    REQUIRE(result);
    return std::move(result).value();
}
template <typename T> void rejected(const gi::Result<T> &result, gi::ErrorCode expected) {
    REQUIRE_FALSE(result);
    CHECK(result.error().code == expected);
}
class ManualClock final : public ghostwire::core::Clock {
  public:
    TimePoint now() const noexcept override { return TimePoint(std::chrono::milliseconds(ticks.load())); }
    std::atomic<std::chrono::milliseconds::rep> ticks{0};
};
constexpr gi::VerificationTime current{1000, true};
struct Harness {
    ManualClock clock;
    std::shared_ptr<TestHooks> hooks = std::make_shared<TestHooks>();
    std::unique_ptr<gc::CryptoProvider> crypto = take(TestAccess::create(hooks));
    std::unique_ptr<gi::IdentityService> service = take(gi::IdentityService::create(*crypto, clock));
    gi::ProfileHandle profile() { return take(service->create_profile("00123", "owned", current)); }
};
} // namespace

TEST_CASE("identity ID20 partial profile creation clears root when second entropy call fails") {
    std::size_t cleared_bytes = 0;
    bool all_cleared = true;
    Harness h;
    h.hooks->on_clear = [&](std::span<const std::uint8_t> bytes) {
        cleared_bytes += bytes.size();
        all_cleared = all_cleared && std::all_of(bytes.begin(), bytes.end(), [](auto b) { return b == 0; });
    };
    unsigned entropy_calls = 0;
    h.hooks->after_entropy = [&] {
        if (++entropy_calls == 1)
            h.hooks->fail_entropy = true;
    };
    rejected(h.service->create_profile("1", "not published", current), gi::ErrorCode::crypto_failure);
    CHECK(entropy_calls == 1);
    CHECK(all_cleared);
    CHECK(cleared_bytes == 64);
    CHECK(h.crypto->diagnostic().live_secret_handles == 0);
    CHECK(h.crypto->diagnostic().live_secret_bytes == 0);
    CHECK(h.service->diagnostic().profiles == 0);
    CHECK(h.service->diagnostic().public_record_bytes == 0);
    h.hooks->fail_entropy = false;
    h.hooks->after_entropy = {};
    CHECK(h.service->create_profile("1", "retry", current));
}

TEST_CASE("identity ID20 staged allocation failure releases private handle reservations") {
    Harness h;
    unsigned entropy_calls = 0;
    h.hooks->after_entropy = [&] {
        if (++entropy_calls == 1)
            h.hooks->fail_allocation = true;
    };
    rejected(h.service->create_profile("1", "allocation failure", current),
             gi::ErrorCode::resource_exhausted);
    CHECK(h.crypto->diagnostic().live_secret_handles == 0);
    CHECK(h.service->diagnostic().profiles == 0);
    CHECK(h.service->diagnostic().private_keys == 0);
    h.hooks->fail_allocation = false;
    h.hooks->after_entropy = {};
    CHECK(h.service->create_profile("1", "retry", current));
}

TEST_CASE("identity ID20 failed membership preparation retains only preexisting independent keys") {
    Harness h;
    const auto p = h.profile();
    auto community_key = take(h.crypto->generate_signing_key());
    const auto c = take(h.service->pin_community(p, take(h.crypto->public_key(community_key))));
    community_key.reset();
    const auto before = take(h.service->profile_record(p));
    unsigned entropy_calls = 0;
    h.hooks->after_entropy = [&] {
        if (++entropy_calls == 1)
            h.hooks->fail_entropy = true;
    };
    rejected(h.service->prepare_membership(p, c, "2", "not admitted", current),
             gi::ErrorCode::crypto_failure);
    CHECK(h.service->diagnostic().memberships == 0);
    CHECK(h.service->diagnostic().private_keys == 2);
    CHECK(h.crypto->diagnostic().live_secret_handles == 2);
    CHECK(take(h.service->profile_record(p)) == before);
    h.hooks->fail_entropy = false;
    h.hooks->after_entropy = {};
    CHECK(h.service->prepare_membership(p, c, "2", "retry", current));
}

TEST_CASE("identity ID20 failed device rotation leaves previous key and signed revision intact") {
    Harness h;
    const auto p = h.profile();
    const auto snapshot = take(h.service->profile(p));
    const auto frame = take(h.service->profile_record(p));
    h.hooks->fail_entropy = true;
    rejected(h.service->rotate_profile_device(p, snapshot.devices[0].id, current),
             gi::ErrorCode::crypto_failure);
    CHECK(take(h.service->profile_record(p)) == frame);
    CHECK(take(h.service->profile(p)).devices[0].id == snapshot.devices[0].id);
    CHECK(h.crypto->diagnostic().live_secret_handles == 2);
    CHECK(h.service->diagnostic().private_keys == 2);
    h.hooks->fail_entropy = false;
    CHECK(h.service->rotate_profile_device(p, snapshot.devices[0].id, current));
    CHECK(h.crypto->diagnostic().live_secret_handles == 2);
}

TEST_CASE("identity ID20 provider busy is immediate rejection with no queued or partial profile") {
    Harness h;
    std::latch entered(1), release(1);
    h.hooks->after_entropy = [&] {
        entered.count_down();
        release.wait();
    };
    std::optional<gc::Result<gc::SecretBuffer>> outcome;
    std::jthread worker([&] { outcome.emplace(h.crypto->random(32)); });
    entered.wait();
    // Always release and join before fatal assertions, including failed expectations.
    const auto busy = h.service->create_profile("1", "busy provider", current);
    release.count_down();
    worker.join();
    h.hooks->after_entropy = {};
    rejected(busy, gi::ErrorCode::busy);
    REQUIRE(outcome.has_value());
    REQUIRE(*outcome);
    outcome.reset();
    CHECK(h.service->diagnostic().profiles == 0);
    CHECK(h.crypto->diagnostic().live_secret_handles == 0);
    CHECK(h.service->create_profile("1", "retry", current));
}

TEST_CASE("identity ID20 owner-thread reentrant operations are rejected without disturbing outer commit") {
    Harness h;
    std::size_t attempts = 0;
    bool rejected_busy = true;
    h.hooks->after_entropy = [&] {
        ++attempts;
        const auto inner = h.service->create_profile("2", "reentrant", current);
        rejected_busy = rejected_busy && !inner && inner.error().code == gi::ErrorCode::busy;
    };
    REQUIRE(h.service->create_profile("1", "outer", current));
    h.hooks->after_entropy = {};
    CHECK(attempts == 2);
    CHECK(rejected_busy);
    CHECK(h.service->diagnostic().profiles == 1);
    CHECK(h.crypto->diagnostic().live_secret_handles == 2);
}

TEST_CASE("identity ID21 cancellation between generated keys never publishes a partial profile") {
    for (unsigned cancel_after : {1U, 2U}) {
        Harness h;
        std::stop_source stop;
        unsigned entropy_calls = 0;
        h.hooks->after_entropy = [&] {
            if (++entropy_calls == cancel_after)
                stop.request_stop();
        };
        rejected(h.service->create_profile("1", "cancelled", current, stop.get_token()),
                 gi::ErrorCode::cancelled);
        CHECK(entropy_calls == cancel_after);
        CHECK(h.service->diagnostic().profiles == 0);
        CHECK(h.service->diagnostic().private_keys == 0);
        CHECK(h.crypto->diagnostic().live_secret_handles == 0);
        h.hooks->after_entropy = {};
        CHECK(h.service->create_profile("1", "retry", current));
    }
}

TEST_CASE("identity ID21 owned stop during provider work cancels publication and cleanup is repeatable") {
    Harness h;
    h.hooks->after_entropy = [&] { h.service->request_stop(); };
    rejected(h.service->create_profile("1", "owned stop", current), gi::ErrorCode::cancelled);
    h.hooks->after_entropy = {};
    CHECK(h.service->diagnostic().profiles == 0);
    CHECK(h.crypto->diagnostic().live_secret_handles == 0);
    REQUIRE(h.service->close());
    REQUIRE(h.service->close());
    // Stopping an identity consumer must not stop its independently owned provider.
    CHECK(h.crypto->generate_signing_key());
}

TEST_CASE("identity ID21 cooperative deadline after entropy discards staged keys and retains old records") {
    Harness h;
    const auto p = h.profile();
    const auto snapshot = take(h.service->profile(p));
    const auto frame = take(h.service->profile_record(p));
    h.hooks->after_entropy = [&] { h.clock.ticks.fetch_add(100); };
    rejected(h.service->rotate_profile_device(p, snapshot.devices[0].id, current),
             gi::ErrorCode::deadline_exceeded);
    h.hooks->after_entropy = {};
    CHECK(take(h.service->profile_record(p)) == frame);
    CHECK(h.service->diagnostic().private_keys == 2);
    CHECK(h.crypto->diagnostic().live_secret_handles == 2);
    CHECK(h.service->rotate_profile_device(p, snapshot.devices[0].id, current));
}

TEST_CASE("identity ID23 provider exceptions are fixed errors and never echo private fixture text") {
    Harness h;
    h.hooks->after_entropy = [] { throw std::runtime_error("SECRET_ALIAS_PATH_KEY_12345"); };
    const auto result = h.service->create_profile("12345", "SECRET_DISPLAY", current);
    rejected(result, gi::ErrorCode::crypto_failure);
    CHECK(gi::describe(result.error().code).find("SECRET") == std::string_view::npos);
    CHECK(gi::describe(result.error().code).find("12345") == std::string_view::npos);
    CHECK(h.service->diagnostic().profiles == 0);
    CHECK(h.crypto->diagnostic().live_secret_handles == 0);
    h.hooks->after_entropy = {};
}

TEST_CASE("identity private key allocations clear exactly once at explicit profile deletion and close") {
    std::size_t clear_count = 0;
    bool all_zero = true;
    Harness h;
    h.hooks->on_clear = [&](std::span<const std::uint8_t> bytes) {
        ++clear_count;
        all_zero = all_zero && bytes.size() == 32 &&
                   std::all_of(bytes.begin(), bytes.end(), [](auto b) { return b == 0; });
    };
    const auto a = h.profile(), b = h.profile();
    CHECK(clear_count == 0);
    REQUIRE(h.service->delete_profile(a));
    CHECK(clear_count == 2);
    CHECK(h.service->profile(b));
    REQUIRE(h.service->close());
    CHECK(clear_count == 4);
    REQUIRE(h.service->close());
    CHECK(clear_count == 4);
    CHECK(all_zero);
    CHECK(h.crypto->diagnostic().live_secret_handles == 0);
}
