#pragma once

#include "ghostwire/core/error.hpp"
#include <optional>
#include <utility>
#include <variant>

namespace ghostwire::core {
template <typename T> class [[nodiscard]] Result {
  public:
    Result(T value) : storage_(std::move(value)) {}
    Result(Error error) : storage_(std::move(error)) {}
    [[nodiscard]] bool has_value() const noexcept { return std::holds_alternative<T>(storage_); }
    explicit operator bool() const noexcept { return has_value(); }
    T &value() & { return std::get<T>(storage_); }
    const T &value() const & { return std::get<T>(storage_); }
    T &&value() && { return std::get<T>(std::move(storage_)); }
    const Error &error() const { return std::get<Error>(storage_); }

  private:
    std::variant<T, Error> storage_;
};

template <> class [[nodiscard]] Result<void> {
  public:
    Result() = default;
    Result(Error error) : error_(std::move(error)) {}
    [[nodiscard]] bool has_value() const noexcept { return !error_.has_value(); }
    explicit operator bool() const noexcept { return has_value(); }
    const Error &error() const { return error_.value(); }

  private:
    std::optional<Error> error_;
};
} // namespace ghostwire::core
