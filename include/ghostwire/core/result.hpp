#pragma once

#include "ghostwire/core/error.hpp"
#include <optional>
#include <utility>
#include <variant>

namespace ghostwire::core {
template <typename T, typename E = Error> class [[nodiscard]] Result {
  public:
    Result(T value) : storage_(std::move(value)) {}
    Result(E error) : storage_(std::move(error)) {}
    [[nodiscard]] bool has_value() const noexcept { return std::holds_alternative<T>(storage_); }
    explicit operator bool() const noexcept { return has_value(); }
    T &value() & { return std::get<T>(storage_); }
    const T &value() const & { return std::get<T>(storage_); }
    T &&value() && { return std::get<T>(std::move(storage_)); }
    const E &error() const { return std::get<E>(storage_); }

  private:
    std::variant<T, E> storage_;
};

template <typename E> class [[nodiscard]] Result<void, E> {
  public:
    Result() = default;
    Result(E error) : error_(std::move(error)) {}
    [[nodiscard]] bool has_value() const noexcept { return !error_.has_value(); }
    explicit operator bool() const noexcept { return has_value(); }
    const E &error() const { return error_.value(); }

  private:
    std::optional<E> error_;
};
} // namespace ghostwire::core
