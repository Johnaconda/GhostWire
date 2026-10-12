#pragma once
#include <initializer_list>
#include <mutex>
#include <ostream>
#include <string_view>

namespace ghostwire::core {
enum class LogLevel { debug, info, warning, error, off };
enum class LogEvent { startup, shutdown, configuration_validated, bootstrap_failed };
enum class LogKey { role, state, checkpoint, error_code, path, address, identity, secret, content, detail };
enum class DataClass { sensitive, public_metadata };
struct LogField {
    LogKey key;
    std::string_view value;
    DataClass classification = DataClass::sensitive;
};

// Sink must outlive this object. Writes are serialized; fields are borrowed only
// for the duration of write(). No arbitrary free-text event messages are accepted.
class Logger {
  public:
    explicit Logger(std::ostream &sink, LogLevel threshold = LogLevel::info);
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    [[nodiscard]] bool write(LogLevel level, LogEvent event, std::initializer_list<LogField> fields = {});
    [[nodiscard]] bool flush();

  private:
    std::ostream &sink_;
    LogLevel threshold_;
    std::mutex mutex_;
};
} // namespace ghostwire::core
