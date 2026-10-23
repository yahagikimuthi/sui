#pragma once

#include <cstdint>
#include <expected>
#include <iostream>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sui {
enum class error_type : std::uint8_t { logic, runtime, count };

inline constexpr auto logic_error   = error_type::logic;
inline constexpr auto runtime_error = error_type::runtime;

class error final {
    struct error_code final {
        error_type  type{error_type::count};
        std::string message;
    };

  public:
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
        codes.assign_range(child_error.codes_);

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
                out += "[Logic Error]: " + code.message + '\n';
            } else {
                out += "[Runtime Error]: " + code.message + '\n';
            }
        }
        return out;
    }

    template <typename E, typename T>
    friend auto operator<<(std::basic_ostream<E, T>& os, const error& self) noexcept
        -> std::basic_ostream<E, T>& {
        for (const auto [i, code] : std::views::enumerate(self.codes_)) {
            if (i != 0) os << " -> ";

            if (code.type == logic_error)
                os << "[Logic Error]: ";
            else
                os << "[Runtimer Error]: ";

            os << code.message << '\n';
        }

        return os;
    }

    void cerr() const noexcept { std::cerr << *this; }

    [[noreturn]] void panic() const noexcept {
        cerr();
        std::terminate();
    }

  private:
    explicit error(std::vector<error_code>&& codes) noexcept : codes_{std::move(codes)} {}

    std::vector<error_code> codes_;
};

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