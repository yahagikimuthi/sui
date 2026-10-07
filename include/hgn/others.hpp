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
}  // namespace hgn