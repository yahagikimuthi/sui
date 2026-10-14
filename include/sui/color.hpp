#pragma once

#include <cstdint>

namespace sui {
struct rgba final {
    std::uint8_t r{};
    std::uint8_t g{};
    std::uint8_t b{};
    std::uint8_t a{255};

    [[nodiscard]] constexpr auto operator+() const noexcept -> rgba { return *this; }

    [[nodiscard]] constexpr auto operator==(const rgba&) const noexcept -> bool = default;
};

inline constexpr auto white   = rgba{.r = 255, .g = 255, .b = 255};
inline constexpr auto silver  = rgba{.r = 192, .g = 192, .b = 192};
inline constexpr auto gray    = rgba{.r = 128, .g = 128, .b = 128};
inline constexpr auto black   = rgba{.r = 0, .g = 0, .b = 0};
inline constexpr auto red     = rgba{.r = 255, .g = 0, .b = 0};
inline constexpr auto maroon  = rgba{.r = 128, .g = 0, .b = 0};
inline constexpr auto yellow  = rgba{.r = 255, .g = 255, .b = 0};
inline constexpr auto olive   = rgba{.r = 128, .g = 128, .b = 0};
inline constexpr auto lime    = rgba{.r = 0, .g = 255, .b = 0};
inline constexpr auto green   = rgba{.r = 0, .g = 128, .b = 0};
inline constexpr auto aqua    = rgba{.r = 0, .g = 255, .b = 255};
inline constexpr auto teal    = rgba{.r = 0, .g = 128, .b = 128};
inline constexpr auto blue    = rgba{.r = 0, .g = 0, .b = 255};
inline constexpr auto navy    = rgba{.r = 0, .g = 0, .b = 128};
inline constexpr auto fuchsia = rgba{.r = 255, .g = 0, .b = 255};
inline constexpr auto purple  = rgba{.r = 128, .g = 0, .b = 128};
}  // namespace sui