#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn {
enum class result : std::int32_t {
    error_out_of_date_khr = VK_ERROR_OUT_OF_DATE_KHR,
    suboptimal_khr        = VK_SUBOPTIMAL_KHR
};

[[nodiscard]] constexpr auto make_unexpected(const VkResult res
) noexcept -> std::unexpected<result> {
    return std::unexpected{static_cast<result>(res)};
}

template <typename T>
[[nodiscard]] constexpr auto make_expected(const VkResult res, T* ptr) noexcept
    -> std::expected<T*, result> {
    if (res != VK_SUCCESS) return make_unexpected(res);
    assert(ptr != nullptr);
    return ptr;
}

[[nodiscard]] constexpr auto make_expected(const VkResult res
) noexcept -> std::expected<void, result> {
    if (res != VK_SUCCESS) return make_unexpected(res);
    return {};
}

enum class queue_flag_bits : std::int32_t { none = 0, graphics = VK_QUEUE_GRAPHICS_BIT };  // NOLINT

[[nodiscard]] constexpr auto operator&(const queue_flag_bits a, const queue_flag_bits b) noexcept
    -> queue_flag_bits {
    const auto raw_a = std::to_underlying(a);
    const auto raw_b = std::to_underlying(b);
    return static_cast<queue_flag_bits>(raw_a & raw_b);
}

enum class surface_transform_flag_bits_khr : std::int32_t {};

enum class format : std::int32_t { b8g8r8a8_srgb = VK_FORMAT_B8G8R8A8_SRGB };  // NOLINT

enum class color_space_khr : std::int32_t {  // NOLINT
    srgb_nonlinear = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
};

enum class present_mode_khr : std::int32_t {  // NOLINT
    fifo    = VK_PRESENT_MODE_FIFO_KHR,
    mailbox = VK_PRESENT_MODE_MAILBOX_KHR
};

enum class sharing_mode : std::int32_t {  // NOLINT
    concurrent = VK_SHARING_MODE_CONCURRENT,
    exclusive  = VK_SHARING_MODE_EXCLUSIVE
};

enum class image_layout : std::int32_t {
    undefined                = VK_IMAGE_LAYOUT_UNDEFINED,
    color_attachment_optimal = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    present_src_khr          = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
};

enum class buffer_usage_flag_bits : std::int32_t {  // NOLINT
    vertex  = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    index   = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
};
}  // namespace hgn