#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <inplace_vector>
#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "akr/enum.hpp"

namespace akr::detail {
class debugger final {
  public:
    template <typename... Args>
    debugger(Args&&...) noexcept = delete;

    static void set(std::function<void(std::string_view)>&& fn) noexcept { fn_ = std::move(fn); }

    static void set_default() noexcept { is_default_ = true; }

    static void reset() noexcept {
        fn_         = {};
        is_default_ = false;
    }

    static void notify(const std::string_view msg) noexcept {
        if (fn_)
            fn_(msg);
        else if (is_default_)
            std::cerr << msg;
    }

  private:
    static inline auto           fn_         = std::function<void(std::string_view)>{};  // NOLINT
    static inline constinit auto is_default_ = false;
};

struct stack_error_code final {
    error_category   category;
    std::string_view msg;
};

struct heap_error_code final {
    error_category category;
    std::string    msg;
};

template <std::size_t N>
class stack_error_impl final {
    friend class heap_error_impl;

  public:
    stack_error_impl(const error_category category, const std::string_view msg) noexcept {
        codes_.try_emplace_back(category, std::string_view{msg});
    }

    template <std::size_t M>
    stack_error_impl(
        const error_category category, const std::string_view msg, const stack_error_impl<M>& parent
    ) noexcept {
        codes_.try_append_range(parent.codes_);
        codes_.try_emplace_back(category, std::string_view{msg});
    }

    [[nodiscard]] auto what() const noexcept -> std::string {
        auto out = std::string{};
        for (const auto [i, code] : std::views::enumerate(codes_)) {
            if (i != 0) out += "->";

            if (code.category == logic_error)
                out += "[Logic Error]: ";
            else if (code.category == runtime_error)
                out += "[Runtime Error]: ";
            else if (code.category == unknown_error)
                out += "[Unknown Error]: ";

            out += code.msg;
            out += '\n';
        }

        return out;
    }

    [[nodiscard]] auto trace_count() const noexcept -> std::uint32_t {
        return static_cast<std::uint32_t>(codes_.size());
    }

  private:
    std::inplace_vector<stack_error_code, N> codes_;
};

class heap_error_impl final {
  public:
    heap_error_impl(const error_category category, const std::string_view msg) noexcept {
        codes_.emplace_back(category, std::string{msg});
    }

    heap_error_impl(
        const error_category category, const std::string_view msg, const heap_error_impl& parent
    ) noexcept {
        codes_.reserve(1 + parent.codes_.size());
        codes_.append_range(parent.codes_);
        codes_.emplace_back(category, std::string{msg});
    }

    template <std::size_t N>
    heap_error_impl(
        const error_category category, const std::string_view msg, const stack_error_impl<N>& parent
    ) noexcept {
        codes_.reserve(1 + parent.trace_count());
        codes_.append_range(
            parent.codes_ |
            std::views::transform([](const stack_error_code& code) noexcept -> heap_error_code {
                return {.category = code.category, .msg = std::string{code.msg}};
            })
        );
        codes_.emplace_back(category, std::string{msg});
    }

    [[nodiscard]] auto what() const noexcept -> std::string {
        auto out = std::string{};
        for (const auto [i, code] : std::views::enumerate(codes_)) {
            if (i != 0) out += "->";

            if (code.category == logic_error)
                out += "[Logic Error]: ";
            else if (code.category == runtime_error)
                out += "[Runtime Error]: ";
            else if (code.category == unknown_error)
                out += "[Unknown Error]: ";

            out += code.msg + '\n';
        }

        return out;
    }

    template <typename E, typename T>
    friend auto operator<<(std::basic_ostream<E, T>& os, const heap_error_impl& self) noexcept
        -> std::basic_ostream<E, T>& {
        os << self.what();
        return os;
    }

    void cerr() const noexcept { std::cerr << *this; }

    [[noreturn]] void panic() const noexcept {
        cerr();
        std::terminate();
    }

    [[nodiscard]] auto trace_count() const noexcept -> std::uint32_t {
        return static_cast<std::uint32_t>(codes_.size());
    }

  private:
    std::vector<heap_error_code> codes_;
};
}  // namespace akr::detail