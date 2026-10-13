#pragma once

#include <compare>

namespace sui {
struct uv final {
    float u{};
    float v{};

    [[nodiscard]] constexpr auto operator+() const noexcept -> uv { return *this; }
    [[nodiscard]] constexpr auto operator-() const noexcept -> uv { return {.u = -u, .v = -v}; }

    [[nodiscard]] constexpr auto operator<=>(const uv&) const noexcept -> auto = default;
    [[nodiscard]] constexpr auto operator==(const uv&) const noexcept -> bool  = default;

    constexpr auto operator+=(const uv other) noexcept -> uv& {
        u += other.u;
        v += other.v;
        return *this;
    }

    constexpr auto operator-=(const uv other) noexcept -> uv& {
        u -= other.u;
        v -= other.v;
        return *this;
    }

    constexpr auto operator*=(const float other) noexcept -> uv& {
        u *= other;
        v *= other;
        return *this;
    }

    constexpr auto operator/=(const float other) noexcept -> uv& {
        u /= other;
        v /= other;
        return *this;
    }

    [[nodiscard]] friend constexpr auto operator+(uv a, const uv b) noexcept -> uv {
        return a += b;
    }

    [[nodiscard]] friend constexpr auto operator-(uv a, const uv b) noexcept -> uv {
        return a -= b;
    }

    [[nodiscard]] friend constexpr auto operator*(uv a, const float b) noexcept -> uv {
        return a *= b;
    }

    [[nodiscard]] friend constexpr auto operator*(const float a, uv b) noexcept -> uv {
        return b * a;
    }

    [[nodiscard]] friend constexpr auto operator/(uv a, const float b) noexcept -> uv {
        return a /= b;
    }
};
}  // namespace sui