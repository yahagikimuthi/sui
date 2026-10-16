#pragma once

#include <cstdint>
#include <expected>
#include <string_view>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn {
inline constexpr auto ext_debug_utils_extension_name =
    std::string_view{VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

enum class queue_flag_bits : std::int32_t {  // NOLINT
    none     = 0,
    graphics = VK_QUEUE_GRAPHICS_BIT,
};

[[nodiscard]] constexpr auto operator&(const queue_flag_bits a, const queue_flag_bits b) noexcept
    -> queue_flag_bits {
    const auto casted_a = std::to_underlying(a);
    const auto casted_b = std::to_underlying(b);
    const auto out      = casted_a & casted_b;
    return static_cast<queue_flag_bits>(out);
}

[[nodiscard]] constexpr auto operator|(const queue_flag_bits a, const queue_flag_bits b) noexcept
    -> queue_flag_bits {
    const auto casted_a = std::to_underlying(a);
    const auto casted_b = std::to_underlying(b);
    const auto out      = casted_a | casted_b;
    return static_cast<queue_flag_bits>(out);
}

enum class format : std::int32_t { b8g8r8a8_srgb = VK_FORMAT_B8G8R8A8_SRGB };  // NOLINT

enum class color_space_khr : std::int32_t {  // NOLINT
    srgb_nonlinear = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
};  // NOLINT

enum class present_mode_khr : std::int32_t {  // NOLINT
    mailbox = VK_PRESENT_MODE_MAILBOX_KHR,
    fifo    = VK_PRESENT_MODE_FIFO_KHR,
};

using extent2d = VkExtent2D;

inline constexpr auto khr_swapchain_extention_name =
    std::string_view{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

enum class surface_transform_flag_bits_khr : std::int32_t {};

enum class result : std::int32_t {
    success               = VK_SUCCESS,
    error_out_of_date_khr = VK_ERROR_OUT_OF_DATE_KHR,
    suboptimal_khr        = VK_SUBOPTIMAL_KHR
};

enum class image_view_type : std::int32_t { type_2d = VK_IMAGE_VIEW_TYPE_2D };  // NOLINT

enum class image_usage_flags : std::int32_t {  // NOLINT
    color_attachment = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
};

enum class sharing_mode : std::int32_t {  // NOLINT
    concurrent = VK_SHARING_MODE_CONCURRENT,
    exclusive  = VK_SHARING_MODE_EXCLUSIVE
};

enum class composite_alpha_flag_bits_khr : std::int32_t {  // NOLINT
    opaque = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
};

enum class command_buffer_level : std::int32_t {  // NOLINT
    primary = VK_COMMAND_BUFFER_LEVEL_PRIMARY
};

enum class image_layout : std::int32_t {  // NOLINT
    undefined                = VK_IMAGE_LAYOUT_UNDEFINED,
    color_attachment_optimal = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    present_src_khr          = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
};

[[nodiscard]] constexpr auto make_result(const VkResult res) noexcept -> std::unexpected<result> {
    return std::unexpected{static_cast<result>(res)};
}
}  // namespace hgn