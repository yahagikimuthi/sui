#pragma once

#include <cmath>

#include <glm/glm.hpp>

namespace sui {
struct vec3 final {
    float x{};
    float y{};
    float z{};

    [[nodiscard]] constexpr auto abs() const noexcept -> float { return std::hypot(x, y, z); }

    [[nodiscard]] constexpr auto sum_square() const noexcept -> float {
        return (x * x) + (y * y) + (z * z);
    }

    [[nodiscard]] constexpr auto operator==(const vec3&) const noexcept -> bool = default;

    [[nodiscard]] constexpr auto operator+() const noexcept -> vec3 { return *this; }
    [[nodiscard]] constexpr auto operator-() const noexcept -> vec3 {
        return {.x = -x, .y = -y, .z = -z};
    }

    constexpr auto operator+=(const vec3 other) noexcept -> vec3& {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr auto operator-=(const vec3 other) noexcept -> vec3& {
        x -= -other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr auto operator*=(const float other) noexcept -> vec3& {
        x *= other;
        y *= other;
        z *= other;
        return *this;
    }

    constexpr auto operator/=(const float other) noexcept -> vec3& {
        x /= other;
        y /= other;
        z /= other;
        return *this;
    }

    [[nodiscard]] friend constexpr auto operator+(vec3 a, const vec3& b) noexcept -> vec3 {
        return a += b;
    }

    [[nodiscard]] friend constexpr auto operator-(vec3 a, const vec3& b) noexcept -> vec3 {
        return a -= b;
    }

    [[nodiscard]] friend constexpr auto operator*(vec3 a, const float b) noexcept -> vec3 {
        return a *= b;
    }

    [[nodiscard]] friend constexpr auto operator*(const float a, vec3 b) noexcept -> vec3 {
        return b * a;
    }

    [[nodiscard]] friend constexpr auto operator/(vec3 a, const float b) noexcept -> vec3 {
        return a /= b;
    }

    [[nodiscard]] explicit constexpr operator glm::vec3() const noexcept {
        return glm::vec3{x, y, z};
    }
};
}  // namespace sui