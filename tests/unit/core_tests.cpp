#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ghostwire/core/application.hpp"
#include "ghostwire/core/bootstrap.hpp"
#include "ghostwire/core/version.hpp"
#include "ghostwire/platform/filesystem.hpp"
#include "ghostwire/platform/platform_info.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <doctest/doctest.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>

using namespace ghostwire;
using namespace ghostwire::core;
namespace fs = std::filesystem;
namespace {
class TemporaryDirectory {
  public:
    TemporaryDirectory() {
        auto base = fs::canonical(fs::temp_directory_path()); // macOS /var may be a symlink.
        const auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
        for (unsigned i = 0; i < 100; ++i) {
            // Keep the host's native TEMP/TMP root and force Unicode/spaces even
            // on ASCII-only profiles. CLI/config text must be explicitly UTF-8.
            path = base / platform::path_from_utf8("ghostwire-test-\xc3\xb6 space-" + std::to_string(seed) +
                                                   "-" + std::to_string(i))
                              .value();
            if (fs::create_directory(path))
                return;
        }
        throw std::runtime_error("Cannot create isolated test directory");
    }
    ~TemporaryDirectory() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
    fs::path path;
};
int cli(std::initializer_list<std::string_view> args, std::string *output = nullptr,
        std::string *errors = nullptr) {
    std::ostringstream out, err;
    const int code = run_cli(Role::cli, {args.begin(), args.size()}, out, err);
    if (output)
        *output = out.str();
    if (errors)
        *errors = err.str();
    return code;
}
Paths test_paths(const fs::path &root) {
    return resolve_paths(Role::cli, root, {}).value();
}
} // namespace

TEST_CASE("version roundtrip and compiled identity") {
    auto version = Version::parse("12.34.56");
    REQUIRE(version);
    CHECK(version.value() == Version{12, 34, 56});
    CHECK(version.value().to_string() == "12.34.56");
    CHECK(Version::parse(version_string()));
    CHECK(checkpoint() == "GW-A03-CRYPTOGRAPHIC-PROVIDER");
}
TEST_CASE("version rejects malformed and overflowing numbers") {
    for (auto value : {"", "1", "1.2", "1.2.3.4", "+1.2.3", "-1.2.3", "01.2.3", "1..3", "1.2.3-alpha",
                       "1.2.3 ", "4294967296.0.0"}) {
        CAPTURE(value);
        CHECK_FALSE(Version::parse(value));
    }
    CHECK(Version::parse("4294967295.0.0"));
}
TEST_CASE("result owns value or structured error including void") {
    Result<std::string> value(std::string("ok"));
    CHECK(value.value() == "ok");
    Result<std::string> failure(Error{ErrorCode::usage, "Bad input."});
    CHECK_FALSE(failure);
    CHECK(exit_code(failure.error().code) == 2);
    CHECK_THROWS_AS(failure.value(), std::bad_variant_access);
    CHECK(Result<void>{});
    CHECK_FALSE(Result<void>{Error{ErrorCode::internal, "Failure."}});
}
TEST_CASE("config defaults are conservative and have no mandatory services") {
    auto parsed = parse_config("");
    REQUIRE(parsed);
    CHECK(parsed.value().privacy_mode == PrivacyMode::protected_mode);
    CHECK(parsed.value().limits.worker_limit == 2);
    CHECK(parsed.value().data_root.empty());
}
TEST_CASE("config accepts whitespace CRLF BOM and explicit budgets") {
    auto parsed = parse_config(
        "\xEF\xBB\xBF# comment\r\n schema_version = "
        "1\r\nlog_level=warning\r\nprivacy_mode=privacy-first\nworker_limit=1\nqueue_limit=16\n");
    REQUIRE(parsed);
    CHECK(parsed.value().privacy_mode == PrivacyMode::privacy_first);
    CHECK(parsed.value().log_level == LogLevel::warning);
    CHECK(parsed.value().limits.worker_limit == 1);
    CHECK(parsed.value().limits.queue_limit == 16);
}
TEST_CASE("config rejects malformed duplicate unknown and unsupported values") {
    for (auto text :
         {"schema_version=2", "schema_version=99999999999999999999999", "worker_limit=0", "worker_limit=65",
          "worker_limit=-1", "worker_limit=+1", "queue_limit=15", "queue_limit=65537", "queue_limit=12x",
          "schema_version=1\nschema_version=1", "bogus=sensitive-text", "log_level=verbose",
          "privacy_mode=unknown", "data_root=relative", "broken", "=value",
          "key=", "worker_limit=2 # inline comments are not supported"}) {
        CAPTURE(text);
        auto result = parse_config(text);
        REQUIRE_FALSE(result);
        CHECK(result.error().code == ErrorCode::configuration);
        CHECK(result.error().message.find("sensitive-text") == std::string::npos);
    }
}
TEST_CASE("config enforces byte budget and rejects embedded control data") {
    CHECK(parse_config(std::string(max_config_bytes, ' ')));
    CHECK_FALSE(parse_config(std::string(max_config_bytes + 1, ' ')));
    CHECK_FALSE(parse_config(std::string("log_level=info\0secret", 21)));
    CHECK_FALSE(parse_config("log_level=info\x1b"));
    CHECK_FALSE(parse_config("log_level=info\x7f"));
}
TEST_CASE("programmatic config validation also enforces enum and schema constraints") {
    Config config;
    config.schema_version = 2;
    CHECK_FALSE(validate_config(config));
    config = Config{};
    config.privacy_mode = static_cast<PrivacyMode>(999);
    CHECK_FALSE(validate_config(config));
    config = Config{};
    config.log_level = static_cast<LogLevel>(999);
    CHECK_FALSE(validate_config(config));
}
TEST_CASE("config loading rejects missing nonregular and oversized files") {
    TemporaryDirectory temp;
    CHECK_FALSE(load_config(temp.path / "absent"));
    CHECK_FALSE(load_config(temp.path));
    const auto file = temp.path / "example.conf";
    {
        std::ofstream out(file);
        out << "worker_limit=3\n";
    }
    auto loaded = load_config(file);
    REQUIRE(loaded);
    CHECK(loaded.value().limits.worker_limit == 3);
    {
        std::ofstream out(file);
        out << std::string(max_config_bytes + 1, 'x');
    }
    auto oversized = load_config(file);
    REQUIRE_FALSE(oversized);
    CHECK(oversized.error().code == ErrorCode::configuration);
}
TEST_CASE("path resolution is pure absolute and role isolated") {
    TemporaryDirectory temp;
    const auto root = temp.path / "not-created";
    auto desktop = resolve_paths(Role::desktop, root, {});
    auto server = resolve_paths(Role::server, root, {});
    REQUIRE(desktop);
    REQUIRE(server);
    CHECK(desktop.value().data == root / "desktop/data");
    CHECK(server.value().data == root / "server/data");
    CHECK_FALSE(fs::exists(root));
    CHECK_FALSE(resolve_paths(Role::cli, "relative", {}));
    CHECK_FALSE(resolve_paths(Role::cli, {}, {}));
    CHECK_FALSE(resolve_paths(static_cast<Role>(99), root, {}));
}
TEST_CASE("OS directory mapping uses independent config and cache roots") {
    TemporaryDirectory temp;
    platform::UserDirectories dirs{temp.path / "d", temp.path / "c", temp.path / "k"};
    auto paths = resolve_paths(Role::relay, {}, dirs);
    REQUIRE(paths);
    CHECK(paths.value().config == dirs.config / "GhostWire/relay");
    CHECK(paths.value().runtime == dirs.cache / "GhostWire/relay/runtime");
    CHECK(initialize_paths(paths.value()));
    CHECK(initialize_paths(paths.value()));
    CHECK(fs::is_directory(paths.value().logs));
}
TEST_CASE("path initialization rejects file collisions") {
    TemporaryDirectory temp;
    {
        std::ofstream out(temp.path / "cli");
        out << "existing content";
    }
    CHECK_FALSE(initialize_paths(test_paths(temp.path)));
    CHECK(fs::is_regular_file(temp.path / "cli"));
}
#ifndef _WIN32
TEST_CASE("path initialization rejects symbolic link traversal") {
    TemporaryDirectory temp;
    fs::create_directory(temp.path / "target");
    fs::create_directory_symlink(temp.path / "target", temp.path / "link");
    CHECK_FALSE(initialize_paths(test_paths(temp.path / "link")));
    CHECK_FALSE(fs::exists(temp.path / "target/cli"));
}
#endif
TEST_CASE("application transitions once and shutdown is idempotent") {
    TemporaryDirectory temp;
    std::ostringstream stream;
    Logger logger(stream);
    Application app(Role::cli, {}, test_paths(temp.path), logger);
    CHECK(app.state() == ApplicationState::created);
    REQUIRE(app.start());
    CHECK(app.state() == ApplicationState::running);
    CHECK_FALSE(app.start());
    CHECK_FALSE(app.stop_token().stop_requested());
    REQUIRE(app.shutdown());
    CHECK(app.state() == ApplicationState::stopped);
    CHECK(app.stop_token().stop_requested());
    CHECK(app.shutdown());
    CHECK_FALSE(app.start());
    CHECK(stream.str().find("startup") != std::string::npos);
    CHECK(stream.str().find("shutdown") != std::string::npos);
    CHECK(stream.str().find(platform::path_to_utf8(temp.path)) == std::string::npos);
}
TEST_CASE("RAII stops an active application and cancellation can be requested across threads") {
    TemporaryDirectory temp;
    std::ostringstream stream;
    Logger logger(stream);
    std::stop_token token;
    {
        Application app(Role::cli, {}, test_paths(temp.path), logger);
        REQUIRE(app.start());
        token = app.stop_token();
        std::jthread requester([&app] { app.request_stop(); });
        requester.join();
        CHECK(token.stop_requested());
    }
    CHECK(stream.str().find("shutdown") != std::string::npos);
}
TEST_CASE("failed initialization stays failed and never reports startup success") {
    TemporaryDirectory temp;
    std::ostringstream stream;
    Logger logger(stream);
    Config invalid;
    invalid.limits.worker_limit = 0;
    Application app(Role::server, invalid, test_paths(temp.path), logger);
    CHECK_FALSE(app.start());
    CHECK(app.state() == ApplicationState::failed);
    CHECK(app.shutdown());
    CHECK_FALSE(fs::exists(temp.path / "cli"));
    CHECK(stream.str().empty());
}
TEST_CASE("shutdown before startup is terminal") {
    TemporaryDirectory temp;
    std::ostringstream stream;
    Logger logger(stream);
    Application app(Role::cli, {}, test_paths(temp.path), logger);
    CHECK(app.shutdown());
    CHECK_FALSE(app.start());
    CHECK_FALSE(fs::exists(temp.path / "cli"));
}
TEST_CASE("logging fails closed for sensitive fields even when incorrectly marked public") {
    std::ostringstream stream;
    Logger logger(stream);
    REQUIRE(logger.write(LogLevel::info, LogEvent::startup,
                         {{LogKey::secret, "NEVER_WRITE_THIS", DataClass::public_metadata},
                          {LogKey::address, "198.51.100.42", DataClass::public_metadata},
                          {LogKey::path, "/private/user"},
                          {LogKey::identity, "identity-key"},
                          {LogKey::content, "message contents"},
                          {LogKey::detail, "token=secret"},
                          {LogKey::role, "cli", DataClass::public_metadata}}));
    const auto output = stream.str();
    CHECK(output.find("NEVER_WRITE_THIS") == std::string::npos);
    CHECK(output.find("198.51") == std::string::npos);
    CHECK(output.find("/private") == std::string::npos);
    CHECK(output.find("identity-key") == std::string::npos);
    CHECK(output.find("message contents") == std::string::npos);
    CHECK(output.find("token=secret") == std::string::npos);
    CHECK(output.find("[REDACTED]") != std::string::npos);
    CHECK(output.find("\"role\":\"cli\"") != std::string::npos);
}
TEST_CASE("logging escapes line injection bounds values and deduplicates fields") {
    std::ostringstream stream;
    Logger logger(stream);
    REQUIRE(logger.write(LogLevel::info, LogEvent::startup,
                         {{LogKey::state, "a\n\"\\\x1b", DataClass::public_metadata},
                          {LogKey::state, "duplicate", DataClass::public_metadata},
                          {LogKey::checkpoint, std::string(10000, 'x'), DataClass::public_metadata}}));
    auto output = stream.str();
    CHECK(std::count(output.begin(), output.end(), '\n') == 1);
    CHECK(output.find("\\u000a") != std::string::npos);
    CHECK(output.find("duplicate") == std::string::npos);
    CHECK(output.size() < 1024);
}
TEST_CASE("logging threshold and stream failures are observable") {
    std::ostringstream stream;
    Logger logger(stream, LogLevel::warning);
    CHECK(logger.write(LogLevel::debug, LogEvent::startup));
    CHECK(stream.str().empty());
    stream.setstate(std::ios::badbit);
    CHECK_FALSE(logger.write(LogLevel::error, LogEvent::bootstrap_failed));
    CHECK_FALSE(logger.flush());
}
TEST_CASE("logging serializes concurrent records") {
    std::ostringstream stream;
    Logger logger(stream);
    std::atomic<bool> good = true;
    std::vector<std::jthread> writers;
    for (int i = 0; i < 4; ++i)
        writers.emplace_back([&] {
            for (int j = 0; j < 50; ++j)
                if (!logger.write(LogLevel::info, LogEvent::startup))
                    good = false;
        });
    writers.clear();
    CHECK(good.load());
    auto records = stream.str();
    CHECK(std::count(records.begin(), records.end(), '\n') == 200);
}
TEST_CASE("CLI returns exact usage config and filesystem codes without echoing raw input") {
    TemporaryDirectory temp;
    std::string out, err;
    CHECK(cli({"--help"}, &out) == 0);
    CHECK(out.find("ghostwire-cli") != std::string::npos);
    CHECK(cli({"--version"}) == 0);
    CHECK(cli({"--check-config"}) == 0);
    CHECK(cli({"--unknown=secret"}, nullptr, &err) == 2);
    CHECK(err.find("secret") == std::string::npos);
    CHECK(cli({"--config"}) == 2);
    CHECK(cli({"--version", "--help"}) == 2);
    CHECK(cli({"--help", "--config", "secret"}) == 2);
    CHECK(cli({"--check-config", "--data-root", "relative"}) == 3);
    const auto absent = platform::path_to_utf8(temp.path / "secret-absent.conf");
    CHECK(cli({"--check-config", "--config", absent}, nullptr, &err) == 4);
    CHECK(err.find("secret-absent") == std::string::npos);
}
TEST_CASE("config check and print paths never initialize storage") {
    TemporaryDirectory temp;
    const auto root = temp.path / "no-writes";
    const auto root_text = platform::path_to_utf8(root);
    CHECK(cli({"--check-config", "--data-root", root_text}) == 0);
    CHECK(cli({"--print-paths", "--data-root", root_text}) == 0);
    CHECK_FALSE(fs::exists(root));
}
TEST_CASE("config precedence is explicit and bootstrap never claims network readiness") {
    TemporaryDirectory temp;
    const auto file = temp.path / "config.conf";
    const auto config_root = temp.path / "config-root";
    {
        std::ofstream config(file);
        config << "data_root=" << platform::path_to_utf8(config_root) << "\nlog_level=off\n";
    }
    const auto file_text = platform::path_to_utf8(file);
    const auto override_root = temp.path / "override";
    const auto override_text = platform::path_to_utf8(override_root);
    std::string output, errors;
    CHECK(cli({"--config", file_text, "--data-root", override_text}, &output, &errors) == 0);
    CHECK_FALSE(fs::exists(config_root));
    CHECK(fs::is_directory(override_root / "cli/data"));
    CHECK(errors.empty());
    CHECK(output.find("no live networking or messaging implemented") != std::string::npos);
}
TEST_CASE("platform diagnostics report local coarse capabilities only") {
    const auto info = platform::platform_info();
    CHECK_FALSE(info.operating_system.empty());
    CHECK_FALSE(info.architecture.empty());
    CHECK(info.logical_processors >= 1);
}
TEST_CASE("UTF-8 paths roundtrip and malformed sequences are rejected") {
    const std::string text = "directory-\xc3\xa5-\xf0\x9f\x91\xbb";
    auto path = platform::path_from_utf8(text);
    REQUIRE(path);
    CHECK(platform::path_to_utf8(path.value()) == text);
    for (auto invalid : {"\xc0\x80", "\xed\xa0\x80", "\xf4\x90\x80\x80", "\x80", "\xe2\x82", "\xe0\x80\x80",
                         "\xff", "\xf6", "x\ny"})
        CHECK_FALSE(platform::path_from_utf8(invalid));
}

TEST_CASE("Unicode profile paths use UTF-8 at CLI and config boundaries under the native temp root") {
    TemporaryDirectory temp;
    const auto profile =
        temp.path / platform::path_from_utf8("Users/J\xc3\xb6rg Owner/AppData/Local/Temp").value();
    REQUIRE(fs::create_directories(profile));
    const auto file = profile / platform::path_from_utf8("settings \xc3\xb6.conf").value();
    const auto root = profile / platform::path_from_utf8("stored data \xc3\xb6").value();
    const auto root_text = platform::path_to_utf8(root);
    const auto file_text = platform::path_to_utf8(file);
    REQUIRE(root_text.find("\xc3\xb6") != std::string::npos);
    REQUIRE(root_text.find(' ') != std::string::npos);
    auto decoded = platform::path_from_utf8(root_text);
    REQUIRE(decoded);
    CHECK(decoded.value() == root);
    {
        std::ofstream config(file, std::ios::binary);
        config << "data_root=" << root_text << '\n';
        REQUIRE(config.good());
    }
    auto loaded = load_config(file);
    REQUIRE(loaded);
    CHECK(loaded.value().data_root == root);
    CHECK(cli({"--check-config", "--config", file_text}) == 0);
    CHECK_FALSE(fs::exists(root));
    std::string output, errors;
    CHECK(cli({"--print-paths", "--config", file_text}, &output, &errors) == 0);
    CHECK(output.find("data=" + platform::path_to_utf8(root / "cli/data")) != std::string::npos);
    CHECK(errors.empty());
    CHECK_FALSE(fs::exists(root));
    CHECK(cli({"--config", file_text}, &output, &errors) == 0);
    CHECK(fs::is_directory(root / "cli/data"));
    CHECK(errors.find(root_text) == std::string::npos);
    CHECK(errors.find("shutdown") != std::string::npos);
}

TEST_CASE("invalid UTF-8 stays rejected in CLI paths and config files without disclosure or writes") {
    TemporaryDirectory temp;
    const auto file = temp.path / "invalid.conf";
    const auto file_text = platform::path_to_utf8(file);
    const auto prefix = platform::path_to_utf8(temp.path / "must-not-be-created");
    for (const auto invalid : {"\xf6", "\xc3", "\xc0\xaf", "\xed\xa0\x80"}) {
        const auto text = prefix + "/private-marker-" + invalid;
        std::string errors;
        CHECK(cli({"--check-config", "--data-root", text}, nullptr, &errors) == 2);
        CHECK(errors.find("private-marker") == std::string::npos);
        CHECK(cli({"--check-config", "--config", text}, nullptr, &errors) == 2);
        CHECK(errors.find("private-marker") == std::string::npos);
        {
            std::ofstream config(file, std::ios::binary);
            config << "data_root=" << text << '\n';
            REQUIRE(config.good());
        }
        CHECK(cli({"--check-config", "--config", file_text}, nullptr, &errors) == 3);
        CHECK(errors.find("private-marker") == std::string::npos);
        CHECK_FALSE(fs::exists(temp.path / "must-not-be-created"));
    }
}

#ifdef _WIN32
TEST_CASE("Windows native UTF-16 username and temp paths convert to UTF-8 without the ANSI code page") {
    const fs::path native(L"C:\\Users\\J\u00f6rg Owner\\AppData\\Local\\Temp\\config \u00f6.conf");
    const auto text = platform::path_to_utf8(native);
    CHECK(text == "C:/Users/J\xc3\xb6rg Owner/AppData/Local/Temp/config \xc3\xb6.conf");
    const auto decoded = platform::path_from_utf8(text);
    REQUIRE(decoded);
    CHECK(decoded.value().lexically_normal() == native.lexically_normal());
}
#endif
