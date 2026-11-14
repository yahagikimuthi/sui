#pragma once

#include <cstddef>
#include <exception>
#include <expected>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include "akr/detail/detail.hpp"
#include "akr/enum.hpp"

namespace akr {
inline void set_debug_fn(std::function<void(std::string_view)> fn) noexcept {
    detail::debugger::set(std::move(fn));
}

inline void use_default_debugger() noexcept {
    detail::debugger::set_default();
}

inline void notify(const std::string_view msg) noexcept {
    detail::debugger::notify(msg);
}

[[nodiscard]] inline auto fail(const std::string_view msg) noexcept -> bool {
    notify(msg);
    return false;
}

class error;

template <std::size_t N>
class stack_error final {
    friend class error;

  public:
    stack_error(const error_category category, const std::string_view msg) noexcept
        : impl_{category, msg} {}

    template <std::size_t M>
    stack_error(
        const error_category category, const std::string_view msg, const stack_error<M>& parent
    ) noexcept
        : impl_{category, msg, parent.impl_} {}

    stack_error(
        const error_category category, const std::string_view msg, const error& parent
    ) noexcept = delete;

    [[nodiscard]] auto what() const noexcept -> std::string { return impl_.what(); }

    template <typename E, typename T>
    friend auto operator<<(std::basic_ostream<E, T>& os, const stack_error& self) noexcept
        -> std::basic_ostream<E, T>& {
        os << self.what();
        return os;
    }

    void cerr() const noexcept { std::cerr << *this; }

    [[noreturn]] void panic() const noexcept {
        cerr();
        std::terminate();
    }

    [[nodiscard]] auto trace_count() const noexcept -> std::uint32_t { return impl_.trace_count(); }

  private:
    detail::stack_error_impl<N> impl_;
};

class error final {
  public:
    error(const error_category category, const std::string_view msg) noexcept
        : impl_{category, msg} {}

    error(const error_category category, const std::string_view msg, const error& parent) noexcept
        : impl_{category, msg, parent.impl_} {}

    template <std::size_t N>
    error(
        const error_category category, const std::string_view msg, const stack_error<N>& parent
    ) noexcept
        : impl_{category, msg, parent.impl_} {}

    [[nodiscard]] auto what() const noexcept -> std::string { return impl_.what(); }

    template <typename E, typename T>
    friend auto operator<<(std::basic_ostream<E, T>& os, const error& self) noexcept
        -> std::basic_ostream<E, T>& {
        os << self.what();
        return os;
    }

    void cerr() const noexcept { std::cerr << *this; }

    [[noreturn]] void panic() const noexcept {
        cerr();
        std::terminate();
    }

    [[nodiscard]] auto trace_count() const noexcept -> std::uint32_t { return impl_.trace_count(); }

  private:
    detail::heap_error_impl impl_;
};

[[nodiscard]] inline auto make_error(
    const error_category category, const std::string_view msg
) noexcept -> std::unexpected<error> {
    return std::unexpected<error>{std::in_place, category, msg};
}

[[nodiscard]] inline auto make_error(
    const error_category category, const std::string_view msg, const error& parent
) noexcept -> std::unexpected<error> {
    return std::unexpected<error>{std::in_place, category, msg, parent};
}

template <std::size_t N>
[[nodiscard]] auto make_error(
    const error_category category, const std::string_view msg, const stack_error<N>& parent
) noexcept -> std::unexpected<error> {
    return std::unexpected<error>{std::in_place, category, msg, parent};
}

template <std::size_t N, std::size_t M = N>
[[nodiscard]] auto make_stack_error(
    const error_category category, const std::string_view msg
) noexcept -> std::unexpected<stack_error<M>> {
    return std::unexpected<stack_error<M>>{std::in_place, category, msg};
}

template <std::size_t N, std::size_t M = N>
[[nodiscard]] auto make_stack_error(
    const error_category category, const std::string_view msg, const stack_error<N>& parent
) noexcept -> std::unexpected<stack_error<M>> {
    return std::unexpected<stack_error<M>>{std::in_place, category, msg, parent};
}

}  // namespace akr