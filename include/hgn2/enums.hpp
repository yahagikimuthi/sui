#pragma once

#include <cstdint>
#include <expected>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn {
enum class result : std::int32_t {};

[[nodiscard]] constexpr auto make_unexpected(const VkResult res) noexcept
    -> std::unexpected<result> {
    return std::unexpected{static_cast<result>(res)};
}

enum class queue_flag_bits : std::int32_t { none = 0, graphics = VK_QUEUE_GRAPHICS_BIT };

[[nodiscard]] constexpr auto operator&(const queue_flag_bits a, const queue_flag_bits b) noexcept
    -> queue_flag_bits {
    const auto raw_a = std::to_underlying(a);
    const auto raw_b = std::to_underlying(b);
    return static_cast<queue_flag_bits>(raw_a & raw_b);
}
}  // namespace hgn