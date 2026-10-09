#pragma once

#include <vulkan/vulkan.h>

#include "hgn/types.hpp"

namespace hgn {
inline constexpr auto* ext_debug_utils_extension_name = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

enum class queue_flag_bits : i32 {
    none             = 0,
    graphics         = VK_QUEUE_GRAPHICS_BIT,
    compute          = VK_QUEUE_COMPUTE_BIT,
    transfer         = VK_QUEUE_TRANSFER_BIT,
    sparse_binding   = VK_QUEUE_SPARSE_BINDING_BIT,
    protected_bit    = VK_QUEUE_PROTECTED_BIT,
    video_decode_khr = VK_QUEUE_VIDEO_DECODE_BIT_KHR,
    video_encode_khr = VK_QUEUE_VIDEO_ENCODE_BIT_KHR,
    optical_flow_nv  = VK_QUEUE_OPTICAL_FLOW_BIT_NV,
    max              = VK_QUEUE_FLAG_BITS_MAX_ENUM
};

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

inline constexpr auto queue_none_bit             = queue_flag_bits::none;
inline constexpr auto queue_graphics_bit         = queue_flag_bits::graphics;
inline constexpr auto queue_compute_bit          = queue_flag_bits::compute;
inline constexpr auto queue_transfer_bit         = queue_flag_bits::transfer;
inline constexpr auto queue_sparse_binding_bit   = queue_flag_bits::sparse_binding;
inline constexpr auto queue_protected_bit        = queue_flag_bits::protected_bit;
inline constexpr auto queue_video_decode_bit_khr = queue_flag_bits::video_decode_khr;
inline constexpr auto queue_video_encode_bit_khr = queue_flag_bits::video_encode_khr;
inline constexpr auto queue_optical_flow_bit_nv  = queue_flag_bits::optical_flow_nv;
inline constexpr auto queue_flag_bits_max_enum   = queue_flag_bits::max;

enum class format : i32 { b8g8r8a8_srgb = VK_FORMAT_B8G8R8A8_SRGB };

inline constexpr auto format_b8g8r8a8_srgb = format::b8g8r8a8_srgb;

enum class color_space_khr : u8 { srgb_nonlinear = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

inline constexpr auto color_space_srgb_nonlinear_khr = color_space_khr::srgb_nonlinear;

enum class present_mode_khr_t : u8 {
    fifo    = VK_PRESENT_MODE_FIFO_KHR,
    mailbox = VK_PRESENT_MODE_MAILBOX_KHR
};

inline constexpr auto present_mode_fifo_khr    = present_mode_khr_t::fifo;
inline constexpr auto present_mode_mailbox_khr = present_mode_khr_t::mailbox;

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