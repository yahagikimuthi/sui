#pragma once

#include <expected>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "sui/detail/others.hpp"
#include "sui/types.hpp"

namespace sui {
enum class error_type : u8 { logic, runtime };

inline constexpr auto logic_error   = error_type::logic;
inline constexpr auto runtime_error = error_type::runtime;

class error final {
    struct error_code final {  // NOLINT
        using str_t = std::variant<std::string_view, std::string>;

        [[nodiscard]] auto message_to_str() const noexcept -> std::string {
            return message.visit([](auto&& str) noexcept -> std::string {
                return static_cast<std::string>(str);
            });
        }
        [[nodiscard]] auto message_to_view() const noexcept -> std::string_view {
            return message.visit([](auto&& str) noexcept -> std::string_view { return str; });
        }

        error_type type;
        str_t      message;
    };

  public:
    template <typename T>
        requires detail::is_string_literal_v<T>
    [[nodiscard]] static auto make(const error_type type, const T& message) noexcept
        -> std::unexpected<error> {
        const auto codes = error_code{.type = type, .message = std::string_view{message}};
        const auto out   = error{std::vector{codes}};
        return std::unexpected{out};
    }

    template <typename T>
        requires detail::is_string_literal_v<T>
    [[nodiscard]] static auto make(
        const error_type type, const T& message, const error& child_error
    ) noexcept -> std::unexpected<error> {
        auto codes = std::vector<error_code>{};
        codes.reserve(1 + child_error.codes_.size());
        codes.emplace_back(type, std::string_view{message});
        codes.append_range(child_error.codes_);

        const auto out = error{std::move(codes)};
        return std::unexpected{out};
    }

    [[nodiscard]] static auto make(const error_type type, const std::string_view message) noexcept
        -> std::unexpected<error> {
        const auto codes = error_code{.type = type, .message = std::string{message}};
        const auto out   = error{std::vector{codes}};
        return std::unexpected{out};
    }

    [[nodiscard]] static auto make(
        const error_type type, const std::string_view message, const error& child_error
    ) noexcept -> std::unexpected<error> {
        auto codes = std::vector<error_code>{};
        codes.reserve(1 + child_error.codes_.size());
        codes.emplace_back(type, std::string{message});
        codes.append_range(child_error.codes_);

        const auto out = error{std::move(codes)};
        return std::unexpected{out};
    }

    [[nodiscard]] auto type() const noexcept -> error_type { return codes_.front().type; }

    [[nodiscard]] auto what() const noexcept -> std::string {
        auto out = std::string{};
        for (const auto [i, code] : std::views::enumerate(codes_)) {
            if (i != 0) {
                out += " -> ";
            }
            if (code.type == logic_error) {
                out += "[Logic Error]: " + code.message_to_str() + '\n';
            } else {
                out += "[Runtime Error]: " + code.message_to_str() + '\n';
            }
        }
        return out;
    }

    void cerr() const noexcept {
        for (const auto [i, code] : std::views::enumerate(codes_)) {
            if (i != 0) {
                std::cerr << " -> ";
            }
            if (code.type == logic_error) {
                std::cerr << "[Logic Error]: " << code.message_to_view() << '\n';
            } else {
                std::cerr << "[Runtime Error]: " << code.message_to_view() << '\n';
            }
        }
    }

    [[noreturn]] void panic() const noexcept {
        cerr();
        std::terminate();
    }

  private:
    explicit error(std::vector<error_code>&& codes) noexcept : codes_{std::move(codes)} {}

    std::vector<error_code> codes_;
};

template <typename T>
    requires detail::is_string_literal_v<T>
[[nodiscard]] inline auto make_error(const error_type type, const T& message) noexcept
    -> std::unexpected<error> {
    return error::make(type, message);
}

template <typename T>
    requires detail::is_string_literal_v<T>
[[nodiscard]] inline auto make_error(
    const error_type type, const T& message, const error& child_error
) noexcept -> std::unexpected<error> {
    return error::make(type, message, child_error);
}

[[nodiscard]] inline auto make_error(const error_type type, const std::string_view message) noexcept
    -> std::unexpected<error> {
    return error::make(type, message);
}

[[nodiscard]] inline auto make_error(
    const error_type type, const std::string_view message, const error& child_error
) noexcept -> std::unexpected<error> {
    return error::make(type, message, child_error);
}
}  // namespace sui