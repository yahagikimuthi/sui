#pragma once

#include <vulkan/vulkan.h>

#include "hgn/types.hpp"

namespace hgn::detail {
[[nodiscard]] constexpr auto make_version(
    const u32 major, const u32 minor, const u32 patch
) noexcept -> u32 {
    return VK_MAKE_VERSION(major, minor, patch);
}

[[nodiscard]] constexpr auto make_api_version(
    u32 variant, const u32 major, const u32 minor, const u32 patch
) noexcept -> u32 {
    return VK_MAKE_API_VERSION(variant, major, minor, patch);
}
}  // namespace hgn::detail