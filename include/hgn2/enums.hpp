#pragma once

#include <cstdint>
#include <expected>

#include <vulkan/vulkan.h>

namespace hgn {
enum class result : std::int32_t {};

[[nodiscard]] constexpr auto make_unexpected(const VkResult res) noexcept
    -> std::unexpected<result> {
    return std::unexpected{static_cast<result>(res)};
}
}  // namespace hgn