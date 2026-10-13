#pragma once

#include <compare>

namespace sui {
struct extent final {
    float width;
    float height;

    [[nodiscard]] constexpr auto square() const noexcept -> float { return width * height; }

    [[nodiscard]] constexpr auto operator+() const noexcept -> extent { return *this; }

    [[nodiscard]] constexpr auto operator<=>(const extent other) const noexcept -> auto {
        return square() <=> other.square();
    }

    [[nodiscard]] constexpr auto operator==(const extent&) const noexcept -> bool = default;

    constexpr auto operator+=(const extent other) noexcept -> extent& {
        width += other.width;
        height += other.height;
        return *this;
    }

    constexpr auto operator-=(const extent other) noexcept -> extent& {
        width -= other.width;
        height -= other.height;
        return *this;
    }

    constexpr auto operator*=(const float other) noexcept -> extent& {
        width *= other;
        height *= other;
        return *this;
    }

    constexpr auto operator/=(const float other) noexcept -> extent& {
        width /= other;
        height /= other;
        return *this;
    }

    [[nodiscard]] friend constexpr auto operator+(extent a, const extent b) noexcept -> extent {
        return a += b;
    }

    [[nodiscard]] friend constexpr auto operator-(extent a, const extent b) noexcept -> extent {
        return a -= b;
    }

    [[nodiscard]] friend constexpr auto operator*(extent a, const float b) noexcept -> extent {
        return a *= b;
    }

    [[nodiscard]] friend constexpr auto operator*(const float a, extent b) noexcept -> extent {
        return b * a;
    }

    [[nodiscard]] friend constexpr auto operator/(extent a, const float b) noexcept -> extent {
        return a /= b;
    }
};
}  // namespace sui