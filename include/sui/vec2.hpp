#pragma once

#include <cmath>
#include <compare>

namespace sui {
struct vec2 final {
    float x{};
    float y{};

    [[nodiscard]] constexpr auto sum_square() const noexcept -> float { return (x * x) + (y * y); }

    [[nodiscard]] constexpr auto abs() const noexcept -> float { return std::hypot(x, y); }

    [[nodiscard]] constexpr auto operator+() const noexcept -> vec2 { return *this; }
    [[nodiscard]] constexpr auto operator-() const noexcept -> vec2 { return {.x = -x, .y = -y}; }

    [[nodiscard]] constexpr auto operator<=>(const vec2&) const noexcept -> auto = default;
    [[nodiscard]] constexpr auto operator==(const vec2&) const noexcept -> bool  = default;

    constexpr auto operator+=(const vec2 other) noexcept -> vec2& {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr auto operator-=(const vec2 other) noexcept -> vec2& {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr auto operator*=(const float other) noexcept -> vec2& {
        x *= other;
        y *= other;
        return *this;
    }

    constexpr auto operator/=(const float other) noexcept -> vec2& {
        x /= other;
        y /= other;
        return *this;
    }

    [[nodiscard]] friend constexpr auto operator+(vec2 a, const vec2& b) noexcept -> vec2 {
        a += b;
        return a;
    }

    [[nodiscard]] friend constexpr auto operator-(vec2 a, const vec2& b) noexcept -> vec2 {
        a -= b;
        return a;
    }

    [[nodiscard]] friend constexpr auto operator*(vec2 a, const float b) noexcept -> vec2 {
        a *= b;
        return a;
    }

    [[nodiscard]] friend constexpr auto operator*(const float a, vec2 b) noexcept -> vec2 {
        return b * a;
    }

    [[nodiscard]] friend constexpr auto operator/(vec2 a, const float b) noexcept -> vec2 {
        a /= b;
        return a;
    }
};
}  // namespace sui