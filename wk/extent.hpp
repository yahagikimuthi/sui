#pragma once

#include <glm/glm.hpp>

namespace wk {
template <typename T>
    requires std::is_arithmetic_v<T>
struct extent final {
    T width;
    T height;

    [[nodiscard]] constexpr auto square() const noexcept -> T { return width * height; }

    [[nodiscard]] constexpr auto operator==(const extent&) const noexcept -> bool = default;

    [[nodiscard]] constexpr auto operator+() const noexcept -> extent { return *this; }

    [[nodiscard]] constexpr auto operator-() const noexcept -> extent {
        return {.width = -width, .height = -height};
    }

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

    constexpr auto operator*=(const T other) noexcept -> extent& {
        width *= other;
        height *= other;
        return *this;
    }

    constexpr auto operator/=(const T other) noexcept -> extent& {
        width /= other;
        height /= other;
        return *this;
    }

    [[nodiscard]] friend constexpr auto operator+(extent a, const extent& b) noexcept -> extent {
        return a += b;
    }

    [[nodiscard]] friend constexpr auto operator-(extent a, const extent& b) noexcept -> extent {
        return a -= b;
    }

    [[nodiscard]] friend constexpr auto operator*(extent a, const T b) noexcept -> extent {
        return a *= b;
    }

    [[nodiscard]] friend constexpr auto operator*(const T a, extent b) noexcept -> extent {
        return b * a;
    }

    [[nodiscard]] friend constexpr auto operator/(extent a, const T b) noexcept -> extent {
        return a /= b;
    }

    [[nodiscard]] explicit constexpr operator glm::vec2() const noexcept {
        return glm::vec2{width, height};
    }
};

using extentf32 = extent<float>;
using extentu32 = extent<std::uint32_t>;
}  // namespace wk