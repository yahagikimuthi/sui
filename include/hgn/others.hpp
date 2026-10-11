#pragma once

#include <vulkan/vulkan.h>

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

enum class present_mode_khr : i32 {
    immediate                 = VK_PRESENT_MODE_IMMEDIATE_KHR,
    mailbox                   = VK_PRESENT_MODE_MAILBOX_KHR,
    fifo                      = VK_PRESENT_MODE_FIFO_KHR,
    fifo_relaxed              = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
    shared_demand_refresh     = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
    shared_continuous_refresh = VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,
    max                       = VK_PRESENT_MODE_MAX_ENUM_KHR
};

inline constexpr auto present_mode_immediate_khr    = present_mode_khr::immediate;
inline constexpr auto present_mode_mailbox_khr      = present_mode_khr::mailbox;
inline constexpr auto present_mode_fifo_khr         = present_mode_khr::fifo;
inline constexpr auto present_mode_fifo_relaxed_khr = present_mode_khr::fifo_relaxed;
inline constexpr auto present_mode_shared_demand_refresh_khr =
    present_mode_khr::shared_demand_refresh;
inline constexpr auto present_mode_continuous_refresh = present_mode_khr::shared_continuous_refresh;

using extent2d = VkExtent2D;

inline constexpr auto* khr_swapchain_extention_name = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

enum class surface_transform_flag_bits_khr : i32 {
    identity                     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    rotate_90                    = VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR,
    rotate_180                   = VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR,
    rotate_270                   = VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR,
    horizontal_mirror            = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR,
    horizontal_mirror_rotate_90  = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR,
    horizontal_mirror_rotate_180 = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR,
    horizontal_mirror_rotate_270 = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR,
    inherit                      = VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR,
    max                          = VK_SURFACE_TRANSFORM_FLAG_BITS_MAX_ENUM_KHR
};

inline constexpr auto surface_transform_identity_bit_khr =
    surface_transform_flag_bits_khr::identity;
inline constexpr auto surface_transform_rotate_90_bit_khr =
    surface_transform_flag_bits_khr::rotate_90;
inline constexpr auto surface_transform_rotate_180_bit_khr =
    surface_transform_flag_bits_khr::rotate_180;
inline constexpr auto surface_transform_rotate_270_bit_khr =
    surface_transform_flag_bits_khr::rotate_270;
inline constexpr auto surface_transform_horizontal_mirror_bit_khr =
    surface_transform_flag_bits_khr::horizontal_mirror;
inline constexpr auto surface_transform_horizontal_mirror_rotate_90_bit_khr =
    surface_transform_flag_bits_khr::horizontal_mirror_rotate_90;
inline constexpr auto surface_transform_horizontal_mirror_rotate_180_bit_khr =
    surface_transform_flag_bits_khr::horizontal_mirror_rotate_180;
inline constexpr auto surface_transform_horizontal_mirror_rotate_270_bit_khr =
    surface_transform_flag_bits_khr::horizontal_mirror_rotate_270;
inline constexpr auto surface_transform_inherit_bit_khr = surface_transform_flag_bits_khr::inherit;
}  // namespace hgn