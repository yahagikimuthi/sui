#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

namespace hgn::detail {
[[nodiscard]] constexpr auto make_version(
    const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
) noexcept -> std::uint32_t {
    return VK_MAKE_VERSION(major, minor, patch);
}

[[nodiscard]] constexpr auto make_api_version(
    std::uint32_t       variant,
    const std::uint32_t major,
    const std::uint32_t minor,
    const std::uint32_t patch
) noexcept -> std::uint32_t {
    return VK_MAKE_API_VERSION(variant, major, minor, patch);
}

static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);
}  // namespace hgn::detail