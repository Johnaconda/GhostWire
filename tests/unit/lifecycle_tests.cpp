#include "ghostwire/core/application.hpp"
#include <algorithm>
#include <atomic>
#include <barrier>
#include <condition_variable>
#include <doctest/doctest.h>
#include <functional>
#include <limits>
#include <sstream>
#include <thread>
#include <type_traits>
#include <vector>

using namespace ghostwire::core;
using namespace std::chrono_literals;
namespace {
class ManualClock final : public Clock {
  public:
    TimePoint now() const noexcept override { return TimePoint(std::chrono::milliseconds(ticks.load())); }
    void advance(std::chrono::milliseconds amount) { ticks.fetch_add(amount.count()); }
    std::atomic<std::chrono::milliseconds::rep> ticks{0};
};
class TraceService final : public Service {
  public:
    TraceService(unsigned id, std::vector<int> &trace) : id_(id), trace_(trace) {}
    ~TraceService() override {
        trace_.push_back(1000 + static_cast<int>(id_));
        if (on_destroy)
            on_destroy();
    }
    Result<void> start(const ApplicationContext &context) override {
        trace_.push_back(static_cast<int>(id_));
        return on_start ? on_start(context) : Result<void>{};
    }
    Result<void> stop(const ShutdownContext &context) override {
        trace_.push_back(-static_cast<int>(id_));
        return on_stop ? on_stop(context) : Result<void>{};
    }
    std::function<Result<void>(const ApplicationContext &)> on_start;
    std::function<Result<void>(const ShutdownContext &)> on_stop;
    std::function<void()> on_destroy;

  private:
    unsigned id_;
    std::vector<int> &trace_;
};
struct Harness {
    std::vector<int> trace;
    std::ostringstream output;
    Logger logger{output};
    ManualClock clock;
    std::stop_source stop;
    ResourceAdmission admission{{2, 4, 64}};
    ApplicationContext context{Role::cli, {}, {}, logger, clock, admission, stop.get_token()};
    ServiceManager manager{context, stop};
    Harness() { trace.reserve(512); }
    TraceService &add(unsigned id, std::initializer_list<unsigned> dependencies = {}) {
        std::vector<ServiceId> deps;
        for (auto dependency : dependencies)
            deps.push_back({dependency});
        auto service = std::make_unique<TraceService>(id, trace);
        auto *observer = service.get();
        REQUIRE(manager.add({id}, std::move(service), deps));
        return *observer;
    }
};
std::vector<unsigned> ids(const ServiceOrder &order) {
    std::vector<unsigned> result;
    for (std::size_t i = 0; i < order.count; ++i)
        result.push_back(order.services[i].value);
    return result;
}
Error fixture_error(ErrorReason reason = ErrorReason::none) {
    return {ErrorCode::internal, "PRIVATE_FIXTURE_CONTENT", reason};
}

// A test-only real worker. It owns its thread, completion state and reservation.
// Cancellation wakes an interruptible wait; stop joins only after completion.
class WaitingWorker final : public Service {
  public:
    explicit WaitingWorker(bool hold_completion = false) : completion_allowed_(!hold_completion) {}
    void release_completion() {
        std::lock_guard lock(mutex_);
        completion_allowed_ = true;
        cv_.notify_all();
    }
    ~WaitingWorker() override {
        release_completion();
        worker_stop_.request_stop();
        cv_.notify_all();
        if (worker_.joinable())
            worker_.join();
    }
    Result<void> start(const ApplicationContext &context) override {
        auto acquired = context.admission().try_acquire(WorkKind::active, 8, context.stop_token());
        if (!acquired)
            return acquired.error();
        lease_ = std::move(acquired).value();
        auto token = context.stop_token();
        worker_ = std::jthread([this, token] {
            std::stop_callback notify(token, [this] { (void)worker_stop_.request_stop(); });
            std::unique_lock lock(mutex_);
            ready_ = true;
            cv_.notify_all();
            cv_.wait(lock, worker_stop_.get_token(), [] { return false; });
            cv_.wait(lock, [this] { return completion_allowed_; });
            lease_.release();
            done_ = true;
            cv_.notify_all();
        });
        std::unique_lock lock(mutex_);
        if (!cv_.wait_for(lock, 2s, [this] { return ready_; }))
            return fixture_error();
        return {};
    }
    Result<void> stop(const ShutdownContext &context) override {
        std::unique_lock lock(mutex_);
        if (!cv_.wait_for(lock, context.remaining(), [this] { return done_; }))
            return fixture_error(ErrorReason::deadline_exceeded);
        lock.unlock();
        if (worker_.joinable())
            worker_.join();
        return {};
    }

  private:
    std::mutex mutex_;
    std::condition_variable_any cv_;
    bool ready_ = false, done_ = false;
    bool completion_allowed_;
    WorkLease lease_;
    std::stop_source worker_stop_;
    std::jthread worker_;
};
} // namespace

TEST_SUITE("lifecycle") {
    TEST_CASE("dependency order is stable across registration permutations and shutdown reverses it") {
        for (const auto &registration :
             {std::vector<unsigned>{4, 3, 2, 1}, std::vector<unsigned>{2, 4, 1, 3}}) {
            Harness h;
            for (auto id : registration) {
                if (id == 4)
                    h.add(id, {2, 3});
                else if (id == 2 || id == 3)
                    h.add(id, {1});
                else
                    h.add(id);
            }
            auto order = h.manager.initialization_order();
            REQUIRE(order);
            CHECK(ids(order.value()) == std::vector<unsigned>{1, 2, 3, 4});
            REQUIRE(h.manager.start());
            CHECK(h.trace == std::vector<int>{1, 2, 3, 4});
            REQUIRE(h.manager.shutdown());
            CHECK(h.trace == std::vector<int>{1, 2, 3, 4, -4, -3, -2, -1});
            CHECK(h.manager.shutdown_report().complete);
        }
    }
    TEST_CASE("missing dependency rejects entire graph before side effects") {
        Harness h;
        h.add(1);
        h.add(2, {99});
        auto result = h.manager.start();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::missing_dependency);
        CHECK(result.error().subject == 2);
        CHECK(result.error().dependency == 99);
        CHECK(h.trace.empty());
        CHECK_FALSE(h.admission.snapshot().accepting);
        CHECK(h.manager.state() == ManagerState::failed);
        CHECK(h.manager.shutdown());
    }
    TEST_CASE("duplicate registration is rejected without replacing the owner") {
        Harness h;
        auto &original = h.add(10);
        bool original_started = false, rejected_destroyed = false;
        original.on_start = [&](const ApplicationContext &) -> Result<void> {
            original_started = true;
            return {};
        };
        auto duplicate = std::make_unique<TraceService>(10, h.trace);
        duplicate->on_destroy = [&] { rejected_destroyed = true; };
        auto result = h.manager.add({10}, std::move(duplicate));
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::duplicate_service);
        CHECK(rejected_destroyed);
        REQUIRE(h.manager.start());
        CHECK(original_started);
        REQUIRE(h.manager.shutdown());
        // Captures remain alive until the manager has destroyed both fixtures.
        original.on_destroy = {};
    }
    TEST_CASE("self cycles and disconnected multi-node cycles prevent all startup") {
        for (bool self : {false, true}) {
            Harness h;
            h.add(1);
            if (self)
                h.add(2, {2});
            else {
                h.add(2, {3});
                h.add(3, {2});
            }
            auto result = h.manager.start();
            REQUIRE_FALSE(result);
            CHECK(result.error().reason == ErrorReason::dependency_cycle);
            CHECK(h.trace.empty());
        }
    }
    TEST_CASE("invalid registrations and graph size are bounded") {
        Harness h;
        CHECK_FALSE(h.manager.add({1}, nullptr));
        CHECK_FALSE(h.manager.add({0}, std::make_unique<TraceService>(0, h.trace)));
        const ServiceId duplicate_deps[]{{1}, {1}};
        CHECK_FALSE(h.manager.add({2}, std::make_unique<TraceService>(2, h.trace), duplicate_deps));
        const ServiceId zero_dep[]{{0}};
        CHECK_FALSE(h.manager.add({2}, std::make_unique<TraceService>(2, h.trace), zero_dep));
        std::array<ServiceId, max_service_dependencies + 1> too_many{};
        CHECK_FALSE(h.manager.add({2}, std::make_unique<TraceService>(2, h.trace), too_many));
        for (unsigned i = 1; i <= max_services; ++i)
            h.add(i);
        CHECK_FALSE(h.manager.add({100}, std::make_unique<TraceService>(100, h.trace)));
        REQUIRE(h.manager.start());
        REQUIRE(h.manager.shutdown());
        CHECK(h.manager.snapshot().count == max_services);
    }
    TEST_CASE("partial initialization is rolled back including the failing service") {
        Harness h;
        h.add(1);
        WorkLease partial;
        auto &second = h.add(2, {1});
        second.on_start = [&](const ApplicationContext &context) -> Result<void> {
            auto acquired = context.admission().try_acquire(WorkKind::active, 32);
            REQUIRE(acquired);
            partial = std::move(acquired).value();
            return fixture_error();
        };
        second.on_stop = [&](const ShutdownContext &context) -> Result<void> {
            CHECK(context.stop.stop_requested());
            CHECK_FALSE(h.admission.snapshot().accepting);
            partial.release();
            return {};
        };
        h.add(3, {2});
        auto result = h.manager.start();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::service_start_failed);
        CHECK(result.error().subject == 2);
        CHECK(result.error().message.find("PRIVATE") == std::string::npos);
        CHECK(h.trace == std::vector<int>{1, 2, -2, -1});
        CHECK(h.admission.snapshot().reserved_bytes == 0);
        CHECK(h.manager.shutdown_report().complete);
        REQUIRE(h.manager.shutdown());
        CHECK(h.trace == std::vector<int>{1, 2, -2, -1});
    }
    TEST_CASE("exceptions are redacted and failed startup still cleans up") {
        Harness h;
        h.add(1);
        h.add(2, {1}).on_start = [](const ApplicationContext &) -> Result<void> {
            throw std::runtime_error("PRIVATE_EXCEPTION_PATH");
        };
        auto result = h.manager.start();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::service_exception);
        CHECK(result.error().message.find("PRIVATE") == std::string::npos);
        CHECK(h.trace == std::vector<int>{1, 2, -2, -1});
        CHECK(h.output.str().empty());
    }
    TEST_CASE("stop failures retain dependencies while independent branches can clean up") {
        Harness h;
        h.add(1);
        h.add(2, {1}).on_stop = [](const ShutdownContext &) -> Result<void> { return fixture_error(); };
        h.add(3);
        REQUIRE(h.manager.start());
        auto result = h.manager.shutdown();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::service_stop_failed);
        CHECK(h.trace == std::vector<int>{1, 2, 3, -3, -2});
        CHECK(h.manager.state() == ManagerState::stopping);
        CHECK_FALSE(h.manager.shutdown_report().complete);
        CHECK(h.manager.shutdown_report().failure_count == 2);
        CHECK(h.manager.shutdown_report().failures[1].reason == ErrorReason::dependency_busy);
        CHECK(h.manager.shutdown_report().failures[1].dependency.value == 2);
        // Do not keep retrying an intentionally failed fixture in its destructor.
        // The owned service destructor is still called before its dependency.
    }
    TEST_CASE("retry completes only unfinished hooks after a transient stop exception") {
        Harness h;
        h.add(1);
        auto &second = h.add(2, {1});
        unsigned attempts = 0;
        second.on_stop = [&](const ShutdownContext &) -> Result<void> {
            if (++attempts == 1)
                throw std::runtime_error("PRIVATE");
            return {};
        };
        h.add(3);
        REQUIRE(h.manager.start());
        REQUIRE_FALSE(h.manager.shutdown());
        CHECK(h.manager.shutdown_report().failures[0].reason == ErrorReason::service_exception);
        REQUIRE(h.manager.shutdown());
        CHECK(h.trace == std::vector<int>{1, 2, 3, -3, -2, -2, -1});
        REQUIRE(h.manager.shutdown());
        CHECK(attempts == 2);
        CHECK(h.manager.state() == ManagerState::stopped);
    }
    TEST_CASE("rollback failure preserves startup error separately from cleanup report") {
        Harness h;
        h.add(1);
        auto &second = h.add(2, {1});
        second.on_start = [](const ApplicationContext &) -> Result<void> { return fixture_error(); };
        second.on_stop = [](const ShutdownContext &) -> Result<void> {
            return fixture_error(ErrorReason::deadline_exceeded);
        };
        auto result = h.manager.start();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::service_start_failed);
        CHECK(h.manager.startup_error()->subject == 2);
        CHECK(h.manager.shutdown_report().failures[0].reason == ErrorReason::deadline_exceeded);
        second.on_stop = {};
        REQUIRE(h.manager.shutdown());
        CHECK(h.manager.state() == ManagerState::failed);
        CHECK(h.manager.shutdown_report().complete);
        CHECK_FALSE(h.manager.start());
    }
    TEST_CASE("shutdown shares one deadline and allows nonblocking cleanup after expiry") {
        Harness h;
        std::vector<Clock::Duration> remaining;
        h.add(1).on_stop = [&](const ShutdownContext &context) -> Result<void> {
            remaining.push_back(context.remaining());
            return {};
        };
        h.add(2, {1}).on_stop = [&](const ShutdownContext &context) -> Result<void> {
            remaining.push_back(context.remaining());
            h.clock.advance(7ms);
            return {};
        };
        h.add(3, {2}).on_stop = [&](const ShutdownContext &context) -> Result<void> {
            remaining.push_back(context.remaining());
            h.clock.advance(7ms);
            return {};
        };
        REQUIRE(h.manager.start());
        auto result = h.manager.shutdown(10ms);
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::deadline_exceeded);
        CHECK(remaining == std::vector<Clock::Duration>{10ms, 3ms, 0ms});
        CHECK(h.manager.shutdown_report().complete);
        CHECK(h.manager.state() == ManagerState::failed);
        const auto trace = h.trace;
        CHECK_FALSE(h.manager.shutdown()); // Persistent completed-with-error report.
        CHECK(h.trace == trace);
    }
    TEST_CASE("a timed out service and its dependencies remain alive for a later deadline") {
        Harness h;
        h.add(1);
        auto &second = h.add(2, {1});
        bool completed = false;
        second.on_stop = [&](const ShutdownContext &context) -> Result<void> {
            if (!completed) {
                h.clock.advance(std::chrono::duration_cast<std::chrono::milliseconds>(context.remaining()));
                return fixture_error(ErrorReason::deadline_exceeded);
            }
            return {};
        };
        REQUIRE(h.manager.start());
        REQUIRE_FALSE(h.manager.shutdown(5ms));
        CHECK(h.trace == std::vector<int>{1, 2, -2});
        CHECK_FALSE(h.manager.shutdown_report().complete);
        completed = true;
        REQUIRE(h.manager.shutdown(10ms));
        CHECK(h.trace == std::vector<int>{1, 2, -2, -2, -1});
    }
    TEST_CASE("stop callbacks consume the same deadline and admission closes before callbacks") {
        Harness h;
        h.add(1);
        bool closed = false;
        std::stop_callback callback(h.context.stop_token(), [&] {
            closed = !h.admission.snapshot().accepting;
            h.clock.advance(11ms);
        });
        REQUIRE(h.manager.start());
        REQUIRE_FALSE(h.manager.shutdown(10ms));
        CHECK(closed);
        CHECK(h.manager.shutdown_report().failures[0].reason == ErrorReason::deadline_exceeded);
    }
    TEST_CASE("invalid deadlines have no side effects and zero deadline permits immediate cleanup") {
        Harness h;
        h.add(1);
        REQUIRE(h.manager.start());
        CHECK_FALSE(h.manager.shutdown(-1ms));
        CHECK_FALSE(h.manager.shutdown(60001ms));
        CHECK(h.manager.state() == ManagerState::running);
        CHECK_FALSE(h.stop.stop_requested());
        REQUIRE(h.manager.shutdown(0ms));
        CHECK(h.trace == std::vector<int>{1, -1});
        CHECK_FALSE(Deadline::after(h.clock, std::chrono::milliseconds::max()));
    }
    TEST_CASE("pre-start cancellation prevents all service hooks") {
        Harness h;
        h.add(1);
        h.manager.request_stop();
        auto result = h.manager.start();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::cancelled);
        CHECK(h.trace.empty());
    }
    TEST_CASE("cancellation during startup rolls back and never starts remaining services") {
        Harness h;
        h.add(1).on_start = [&](const ApplicationContext &) -> Result<void> {
            h.manager.request_stop();
            return {};
        };
        h.add(2, {1});
        auto result = h.manager.start();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::cancelled);
        CHECK(h.trace == std::vector<int>{1, -1});
    }
    TEST_CASE("lifecycle reentrancy and mutation from hooks are rejected") {
        Harness h;
        h.add(1).on_start = [&](const ApplicationContext &) -> Result<void> {
            CHECK_FALSE(h.manager.start());
            CHECK_FALSE(h.manager.shutdown());
            CHECK_FALSE(h.manager.initialization_order());
            CHECK_FALSE(h.manager.add({2}, std::make_unique<TraceService>(2, h.trace)));
            return {};
        };
        REQUIRE(h.manager.start());
        CHECK_FALSE(h.manager.add({3}, std::make_unique<TraceService>(3, h.trace)));
        REQUIRE(h.manager.shutdown());
        CHECK_FALSE(h.manager.start());
    }
    TEST_CASE("shutdown before startup is terminal with no start or stop hooks") {
        Harness h;
        h.add(1);
        REQUIRE(h.manager.shutdown());
        REQUIRE(h.manager.shutdown());
        CHECK(h.trace.empty());
        CHECK_FALSE(h.manager.start());
        CHECK_FALSE(h.manager.add({2}, std::make_unique<TraceService>(2, h.trace)));
    }
    TEST_CASE("context and manager must use the same cancellation state") {
        Harness h;
        std::stop_source other;
        ServiceManager mismatched(h.context, other);
        auto result = mismatched.start();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::invalid_registration);
    }
    TEST_CASE("manager owns services and destroys them in reverse dependency order") {
        std::vector<int> destruction;
        bool dependency_alive = true, observed_alive = false, context_alive = false;
        {
            Harness h;
            auto &dependent = h.add(2, {1});
            auto &dependency = h.add(1);
            dependency.on_destroy = [&] {
                dependency_alive = false;
                destruction.push_back(1);
            };
            dependent.on_destroy = [&] {
                observed_alive = dependency_alive;
                context_alive = h.context.role() == Role::cli;
                destruction.push_back(2);
            };
            REQUIRE(h.manager.start());
            REQUIRE(h.manager.shutdown());
            CHECK(destruction.empty()); // Stop does not destroy injected dependencies.
        }
        CHECK(destruction == std::vector<int>{2, 1});
        CHECK(observed_alive);
        CHECK(context_alive);
    }
    TEST_CASE("RAII also destroys a never-started valid graph in dependency order") {
        std::vector<int> destruction;
        {
            Harness h;
            h.add(2, {1}).on_destroy = [&] { destruction.push_back(2); };
            h.add(1).on_destroy = [&] { destruction.push_back(1); };
        }
        CHECK(destruction == std::vector<int>{2, 1});
    }
    TEST_CASE("diagnostics are bounded copied structural snapshots without private text") {
        Harness h;
        h.add(3);
        h.add(1);
        auto before = h.manager.snapshot();
        CHECK(before.services[0].id.value == 1);
        REQUIRE(h.manager.start());
        auto running = h.manager.snapshot();
        CHECK(before.state == ManagerState::created);
        CHECK(running.state == ManagerState::running);
        CHECK(running.services[0].cleanup_pending);
        REQUIRE(h.manager.shutdown());
        CHECK(running.services[0].state == ServiceState::running);
        CHECK(h.manager.snapshot().services[0].state == ServiceState::stopped);
    }
    TEST_CASE("escaped work prevents a false clean shutdown until its lease is released") {
        Harness h;
        REQUIRE(h.manager.start());
        auto lease = h.admission.try_acquire(WorkKind::queued, 1);
        REQUIRE(lease);
        auto result = h.manager.shutdown();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::outstanding_work);
        CHECK_FALSE(h.manager.shutdown_report().complete);
        lease.value().release();
        REQUIRE(h.manager.shutdown());
    }
    TEST_CASE("cooperative cancellation wakes and joins a real owned worker") {
        std::ostringstream output;
        Logger logger(output);
        SteadyClock clock;
        std::stop_source stop;
        ResourceAdmission admission({1, 1, 16});
        ApplicationContext context(Role::server, {}, {}, logger, clock, admission, stop.get_token());
        ServiceManager manager(context, stop);
        REQUIRE(manager.add({1}, std::make_unique<WaitingWorker>()));
        REQUIRE(manager.start());
        CHECK(admission.snapshot().active == 1);
        std::jthread requester([&] { manager.request_stop(); });
        requester.join();
        REQUIRE(manager.shutdown(2000ms));
        CHECK(stop.stop_requested());
        CHECK(admission.snapshot().reserved_bytes == 0);
        CHECK(manager.shutdown_report().complete);
    }
    TEST_CASE("application context takes immutable snapshots and owns built-in filesystem lifecycle") {
        std::ostringstream output;
        Logger logger(output);
        ManualClock clock;
        Config config;
        Paths paths;
        config.limits.worker_limit = 4;
        paths.data = "original";
        Application app(Role::relay, config, paths, logger, clock);
        config.limits.worker_limit = 50;
        paths.data = "changed";
        CHECK(app.context().config().limits.worker_limit == 4);
        CHECK(app.context().paths().data == "original");
        CHECK(&app.context().logger() == &logger);
        CHECK(&app.context().clock() == &clock);
        CHECK(app.services().count == 1);
        CHECK(app.services().services[0].id == application_paths_service);
        static_assert(std::is_const_v<std::remove_reference_t<decltype(app.context().config())>>);
        static_assert(std::is_const_v<std::remove_reference_t<decltype(app.context().paths())>>);
    }
    TEST_CASE("application validates graph before directory initialization") {
        std::ostringstream output;
        Logger logger(output);
        std::vector<int> trace;
        Application app(Role::cli, {}, {}, logger);
        const ServiceId missing[]{{123}};
        REQUIRE(app.add_service({2}, std::make_unique<TraceService>(2, trace), missing));
        auto result = app.start();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::missing_dependency);
        CHECK(trace.empty());
        CHECK(output.str().empty());
        CHECK(app.shutdown());
    }
}

TEST_SUITE("admission") {
    TEST_CASE("CPU headroom clamps active slots and supports unknown and single CPU hosts") {
        CHECK(resource_budget({64, 16}, 12).active_limit == 9);
        CHECK(resource_budget({2, 16}, 12).active_limit == 2);
        CHECK(resource_budget({64, 16}, 1).active_limit == 1);
        CHECK(resource_budget({64, 16}, 0).active_limit == 1);
        CHECK(resource_budget({64, 16}, 8, {64, 50}).active_limit == 4);
        CHECK(resource_budget({64, 16}, 8, {64, 0}).active_limit == 8);
        CHECK(resource_budget({64, 16}, std::numeric_limits<unsigned>::max()).active_limit == 64);
        CHECK(resource_budget({0, 16}, 8).active_limit == 0);
        CHECK(resource_budget({2, 16}, 8, {64, 100}).active_limit == 0);
        CHECK(resource_budget({2, 16}, 8, {0, 20}).active_limit == 0);
    }
    TEST_CASE("active queued and memory caps reject work without changing reserved resources") {
        ResourceAdmission admission({1, 2, 10});
        REQUIRE(admission.open());
        auto active = admission.try_acquire(WorkKind::active, 6);
        REQUIRE(active);
        CHECK_FALSE(admission.try_acquire(WorkKind::active, 0));
        auto queued = admission.try_acquire(WorkKind::queued, 4);
        REQUIRE(queued);
        auto rejected = admission.try_acquire(WorkKind::queued, 1);
        REQUIRE_FALSE(rejected);
        CHECK(rejected.error().reason == ErrorReason::resource_exhausted);
        auto empty_payload = admission.try_acquire(WorkKind::queued, 0);
        REQUIRE(empty_payload);
        CHECK_FALSE(admission.try_acquire(WorkKind::queued, 0));
        auto snapshot = admission.snapshot();
        CHECK(snapshot.active == 1);
        CHECK(snapshot.queued == 2);
        CHECK(snapshot.reserved_bytes == 10);
        CHECK(snapshot.accepted == 3);
        CHECK(snapshot.rejected == 3);
    }
    TEST_CASE("promotion is atomic and preserves memory accounting under backpressure") {
        ResourceAdmission admission({1, 2, 20});
        REQUIRE(admission.open());
        auto active = admission.try_acquire(WorkKind::active, 10);
        auto queued = admission.try_acquire(WorkKind::queued, 5);
        REQUIRE(active);
        REQUIRE(queued);
        CHECK_FALSE(queued.value().try_activate());
        CHECK(admission.snapshot().queued == 1);
        active.value().release();
        REQUIRE(queued.value().try_activate());
        CHECK(admission.snapshot().active == 1);
        CHECK(admission.snapshot().queued == 0);
        CHECK(admission.snapshot().reserved_bytes == 5);
        CHECK_FALSE(queued.value().try_activate());
    }
    TEST_CASE("leases move safely and release exactly once including move assignment") {
        ResourceAdmission admission({2, 2, 20});
        REQUIRE(admission.open());
        auto first = admission.try_acquire(WorkKind::active, 5);
        auto second = admission.try_acquire(WorkKind::queued, 6);
        REQUIRE(first);
        REQUIRE(second);
        WorkLease owned = std::move(first).value();
        CHECK_FALSE(first.value());
        owned = std::move(second).value();
        CHECK(admission.snapshot().active == 0);
        CHECK(admission.snapshot().queued == 1);
        CHECK(admission.snapshot().reserved_bytes == 6);
        owned.release();
        owned.release();
        CHECK(admission.snapshot().reserved_bytes == 0);
        CHECK_FALSE(owned.try_activate());
    }
    TEST_CASE("lease release is safe after its owner is destroyed") {
        WorkLease lease;
        {
            ResourceAdmission admission({1, 1, 1});
            REQUIRE(admission.open());
            auto acquired = admission.try_acquire(WorkKind::queued, 1);
            REQUIRE(acquired);
            lease = std::move(acquired).value();
        }
        auto promoted = lease.try_activate();
        REQUIRE_FALSE(promoted);
        CHECK(promoted.error().reason == ErrorReason::admission_closed);
        lease.release();
    }
    TEST_CASE("cancellation and close reject new work and queued promotion") {
        ResourceAdmission admission({1, 2, 10});
        CHECK_FALSE(admission.try_acquire(WorkKind::active, 0));
        REQUIRE(admission.open());
        CHECK_FALSE(admission.open());
        auto queued = admission.try_acquire(WorkKind::queued, 1);
        REQUIRE(queued);
        std::stop_source stop;
        stop.request_stop();
        CHECK_FALSE(admission.try_acquire(WorkKind::active, 1, stop.get_token()));
        CHECK_FALSE(queued.value().try_activate(stop.get_token()));
        admission.close();
        admission.close();
        CHECK_FALSE(admission.try_acquire(WorkKind::active, 0));
        CHECK_FALSE(queued.value().try_activate());
        CHECK_FALSE(admission.open());
    }
    TEST_CASE("close before open is terminal and invalid budgets never admit work") {
        ResourceAdmission closed({1, 1, 1});
        closed.close();
        CHECK_FALSE(closed.open());
        for (auto budget : {ResourceBudget{0, 1, 1}, ResourceBudget{1, 0, 1}, ResourceBudget{1, 1, 0},
                            ResourceBudget{65, 1, 1}, ResourceBudget{1, 65537, 1}}) {
            ResourceAdmission admission(budget);
            CHECK_FALSE(admission.open());
            CHECK_FALSE(admission.try_acquire(WorkKind::queued, 0));
        }
    }
    TEST_CASE("reservation arithmetic rejects overflow at size_t maximum") {
        const auto maximum = std::numeric_limits<std::size_t>::max();
        ResourceAdmission admission({1, 2, maximum});
        REQUIRE(admission.open());
        auto first = admission.try_acquire(WorkKind::queued, maximum - 1);
        REQUIRE(first);
        CHECK_FALSE(admission.try_acquire(WorkKind::queued, 2));
        auto last = admission.try_acquire(WorkKind::active, 1);
        REQUIRE(last);
        CHECK(admission.snapshot().reserved_bytes == maximum);
        first.value().release();
        last.value().release();
        CHECK(admission.snapshot().reserved_bytes == 0);
    }
    TEST_CASE("parallel admission stays within limits and returns all reservations") {
        ResourceAdmission admission({2, 4, 32});
        REQUIRE(admission.open());
        std::barrier start(5);
        std::atomic<bool> valid{true};
        std::vector<std::jthread> workers;
        for (int i = 0; i < 4; ++i) {
            workers.emplace_back([&] {
                start.arrive_and_wait();
                for (int n = 0; n < 1000; ++n) {
                    auto lease = admission.try_acquire(WorkKind::active, 16);
                    auto snapshot = admission.snapshot();
                    if (snapshot.active > 2 || snapshot.reserved_bytes > 32 || snapshot.queued != 0)
                        valid = false;
                }
            });
        }
        start.arrive_and_wait();
        workers.clear();
        auto snapshot = admission.snapshot();
        CHECK(valid.load());
        CHECK(snapshot.active == 0);
        CHECK(snapshot.reserved_bytes == 0);
        CHECK(snapshot.accepted + snapshot.rejected == 4000);
    }
}

TEST_SUITE("lifecycle") {
    TEST_CASE("real worker timeout returns with ownership retained and retry joins it") {
        std::ostringstream output;
        Logger logger(output);
        SteadyClock clock;
        std::stop_source stop;
        ResourceAdmission admission({1, 1, 16});
        ApplicationContext context(Role::server, {}, {}, logger, clock, admission, stop.get_token());
        std::vector<int> trace;
        trace.reserve(8);
        ServiceManager manager(context, stop);
        REQUIRE(manager.add({1}, std::make_unique<TraceService>(1, trace)));
        auto worker = std::make_unique<WaitingWorker>(true);
        auto *owned_worker = worker.get();
        const ServiceId deps[]{{1}};
        REQUIRE(manager.add({2}, std::move(worker), deps));
        REQUIRE(manager.start());
        const auto before = std::chrono::steady_clock::now();
        auto result = manager.shutdown(10ms);
        const auto elapsed = std::chrono::steady_clock::now() - before;
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::deadline_exceeded);
        CHECK(elapsed < 2s); // Generous scheduling tolerance, not a throughput benchmark.
        CHECK_FALSE(manager.shutdown_report().complete);
        CHECK(trace == std::vector<int>{1});
        CHECK(admission.snapshot().active == 1);
        owned_worker->release_completion();
        REQUIRE(manager.shutdown(2000ms));
        CHECK(trace == std::vector<int>{1, -1});
        CHECK(admission.snapshot().active == 0);
    }
    TEST_CASE("deadline addition saturates near the steady clock representation limit") {
        class NearLimitClock final : public Clock {
          public:
            TimePoint now() const noexcept override { return TimePoint::max() - 500us; }
        } clock;
        auto deadline = Deadline::after(clock, 1ms);
        REQUIRE(deadline);
        CHECK(deadline.value().time_point() == Clock::TimePoint::max());
        CHECK(deadline.value().remaining(clock) == 500us);
    }
    TEST_CASE("application composes filesystem dependency and owned service work in one lifecycle") {
        struct Directory {
            std::filesystem::path path;
            Directory() {
                const auto base = std::filesystem::canonical(std::filesystem::temp_directory_path());
                const auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
                for (unsigned i = 0; i < 100; ++i) {
                    path = base / ("ghostwire-a01-" + std::to_string(seed) + "-" + std::to_string(i));
                    if (std::filesystem::create_directory(path))
                        return;
                }
                throw std::runtime_error("Cannot create test directory");
            }
            ~Directory() {
                std::error_code ec;
                std::filesystem::remove_all(path, ec);
            }
        } directory;
        std::ostringstream output;
        Logger logger(output);
        auto resolved = resolve_paths(Role::cli, directory.path / "data", {});
        REQUIRE(resolved);
        std::vector<int> trace;
        WorkLease lease;
        Application app(Role::cli, {}, resolved.value(), logger);
        auto service = std::make_unique<TraceService>(2, trace);
        service->on_start = [&](const ApplicationContext &context) -> Result<void> {
            CHECK(std::filesystem::is_directory(context.paths().runtime));
            auto acquired = context.admission().try_acquire(WorkKind::active, 1024, context.stop_token());
            REQUIRE(acquired);
            lease = std::move(acquired).value();
            return {};
        };
        service->on_stop = [&](const ShutdownContext &context) -> Result<void> {
            CHECK(context.stop.stop_requested());
            lease.release();
            return {};
        };
        const ServiceId deps[]{application_paths_service};
        REQUIRE(app.add_service({2}, std::move(service), deps));
        REQUIRE(app.start());
        CHECK(app.services().admission.active == 1);
        REQUIRE(app.shutdown());
        CHECK(app.shutdown_report().complete);
        CHECK(app.services().admission.reserved_bytes == 0);
        CHECK(trace == std::vector<int>{2, -2});
        CHECK(std::filesystem::is_directory(resolved.value().data));
        const auto state = app.state();
        CHECK_FALSE(app.shutdown(-1ms));
        CHECK(app.state() == state);
    }
    TEST_CASE("invalid application resource policy is rejected before path initialization") {
        std::ostringstream output;
        Logger logger(output);
        ApplicationOptions options;
        options.resources.memory_budget_bytes = 0;
        Application app(Role::cli, {}, {}, logger, options);
        auto result = app.start();
        REQUIRE_FALSE(result);
        CHECK(result.error().reason == ErrorReason::invalid_budget);
        CHECK(result.error().code == ErrorCode::configuration);
        CHECK(output.str().empty());
        CHECK(app.shutdown());
    }
}
