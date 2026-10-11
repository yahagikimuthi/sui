#pragma once

#include <vulkan/vulkan.h>
#include <expected>

#include "hgn/types.hpp"

namespace hgn {
inline constexpr auto* ext_debug_utils_extension_name = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

enum class queue_flag_bits : i32 {  // NOLINT
    none     = 0,
    graphics = VK_QUEUE_GRAPHICS_BIT,
};
inline constexpr auto queue_none_bit     = queue_flag_bits::none;
inline constexpr auto queue_graphics_bit = queue_flag_bits::graphics;

[[nodiscard]] constexpr auto operator&(const queue_flag_bits a, const queue_flag_bits b) noexcept
    -> queue_flag_bits {
    const auto casted_a = static_cast<i32>(a);
    const auto casted_b = static_cast<i32>(b);
    const auto out      = casted_a & casted_b;
    return static_cast<queue_flag_bits>(out);
}

[[nodiscard]] constexpr auto operator|(const queue_flag_bits a, const queue_flag_bits b) noexcept
    -> queue_flag_bits {
    const auto casted_a = static_cast<i32>(a);
    const auto casted_b = static_cast<i32>(b);
    const auto out      = casted_a | casted_b;
    return static_cast<queue_flag_bits>(out);
}

enum class format : i32 { b8g8r8a8_srgb = VK_FORMAT_B8G8R8A8_SRGB };  // NOLINT
inline constexpr auto format_b8g8r8a8_srgb = format::b8g8r8a8_srgb;

enum class color_space_khr : i32 { srgb_nonlinear = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };  // NOLINT
inline constexpr auto color_space_srgb_nonlinear_khr = color_space_khr::srgb_nonlinear;

enum class present_mode_khr : i32 {  // NOLINT
    mailbox = VK_PRESENT_MODE_MAILBOX_KHR,
    fifo    = VK_PRESENT_MODE_FIFO_KHR,
};
inline constexpr auto present_mode_mailbox_khr = present_mode_khr::mailbox;
inline constexpr auto present_mode_fifo_khr    = present_mode_khr::fifo;

using extent2d = VkExtent2D;

inline constexpr auto* khr_swapchain_extention_name = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

enum class surface_transform_flag_bits_khr : i32 {};

enum class result : i32 {};

enum class image_view_type : i32 { type_2d = VK_IMAGE_VIEW_TYPE_2D };  // NOLINT
inline constexpr auto image_view_type_2d = image_view_type::type_2d;

enum class command_buffer_level : u8 {
    primary   = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY
};
inline constexpr auto command_buffer_level_primary   = command_buffer_level::primary;
inline constexpr auto command_buffer_level_secondary = command_buffer_level::secondary;

[[nodiscard]] constexpr auto make_result(const VkResult res) noexcept -> std::unexpected<result> {
    return std::unexpected{static_cast<result>(res)};
}
}  // namespace hgn