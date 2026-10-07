#pragma once

#include <vulkan/vulkan.h>

#include "hgn/types.hpp"

namespace hgn {
enum class result : i32 {
    success                                      = VK_SUCCESS,
    not_ready                                    = VK_NOT_READY,
    timeout                                      = VK_TIMEOUT,
    event_set                                    = VK_EVENT_SET,
    event_reset                                  = VK_EVENT_RESET,
    incomplete                                   = VK_INCOMPLETE,
    error_out_of_host_memory                     = VK_ERROR_OUT_OF_HOST_MEMORY,
    error_out_of_device_memory                   = VK_ERROR_OUT_OF_DEVICE_MEMORY,
    error_initialization_failed                  = VK_ERROR_INITIALIZATION_FAILED,
    error_device_lost                            = VK_ERROR_DEVICE_LOST,
    error_memory_map_failed                      = VK_ERROR_MEMORY_MAP_FAILED,
    error_layer_not_present                      = VK_ERROR_LAYER_NOT_PRESENT,
    error_extension_not_present                  = VK_ERROR_EXTENSION_NOT_PRESENT,
    error_feature_not_present                    = VK_ERROR_FEATURE_NOT_PRESENT,
    error_incompatible_driver                    = VK_ERROR_INCOMPATIBLE_DRIVER,
    error_too_many_objects                       = VK_ERROR_TOO_MANY_OBJECTS,
    error_format_not_supported                   = VK_ERROR_FORMAT_NOT_SUPPORTED,
    error_fragmented_pool                        = VK_ERROR_FRAGMENTED_POOL,
    error_unknown                                = VK_ERROR_UNKNOWN,
    error_out_of_pool_memory                     = VK_ERROR_OUT_OF_POOL_MEMORY,
    error_invalid_external_handle                = VK_ERROR_INVALID_EXTERNAL_HANDLE,
    error_fragmentation                          = VK_ERROR_FRAGMENTATION,
    error_invalid_opaque_capture_address         = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
    pipeline_compile_required                    = VK_PIPELINE_COMPILE_REQUIRED,
    error_surface_lost_khr                       = VK_ERROR_SURFACE_LOST_KHR,
    error_native_window_in_use_khr               = VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
    suboptimal_khr                               = VK_SUBOPTIMAL_KHR,
    error_out_of_date_khr                        = VK_ERROR_OUT_OF_DATE_KHR,
    error_incompatible_display_khr               = VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,
    error_validation_failed_ext                  = VK_ERROR_VALIDATION_FAILED_EXT,
    error_invalid_shader_nv                      = VK_ERROR_INVALID_SHADER_NV,
    error_image_usage_not_supported_khr          = VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR,
    error_video_picture_layout_not_supported_khr = VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR,
    error_video_profile_operation_not_supported_khr =
        VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR,
    error_video_profile_format_not_supported_khr = VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR,
    error_video_profile_codec_not_supported_khr  = VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR,
    error_video_std_version_not_supported_khr    = VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR,
    error_invalid_drm_format_modifier_plane_layout_ext =
        VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT,
    error_not_permitted_khr                   = VK_ERROR_NOT_PERMITTED_KHR,
    error_full_screen_exclusive_mode_lost_ext = VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
    thread_idle_khr                           = VK_THREAD_IDLE_KHR,
    thread_done_khr                           = VK_THREAD_DONE_KHR,
    operation_deferred_khr                    = VK_OPERATION_DEFERRED_KHR,
    operation_not_deferred_khr                = VK_OPERATION_NOT_DEFERRED_KHR,
    error_invalid_video_std_parameters_khr    = VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR,
    error_compression_exhausted_ext           = VK_ERROR_COMPRESSION_EXHAUSTED_EXT,
    error_incompatible_shader_binary_ext      = VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT,
    error_out_of_pool_memory_khr              = VK_ERROR_OUT_OF_POOL_MEMORY_KHR,
    error_invalid_external_handle_khr         = VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR,
    error_fragmentation_ext                   = VK_ERROR_FRAGMENTATION_EXT,
    error_not_permitted_ext                   = VK_ERROR_NOT_PERMITTED_KHR,
    error_invalid_device_address_ext          = VK_ERROR_INVALID_DEVICE_ADDRESS_EXT,
    error_invalid_opaque_capture_address_khr  = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
    pipeline_compile_required_ext             = VK_PIPELINE_COMPILE_REQUIRED_EXT,
    error_pipeline_compile_required_ext       = VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT
};

using result::error_compression_exhausted_ext;
using result::error_device_lost;
using result::error_extension_not_present;
using result::error_feature_not_present;
using result::error_format_not_supported;
using result::error_fragmentation;
using result::error_fragmentation_ext;
using result::error_fragmented_pool;
using result::error_full_screen_exclusive_mode_lost_ext;
using result::error_image_usage_not_supported_khr;
using result::error_incompatible_display_khr;
using result::error_incompatible_driver;
using result::error_incompatible_shader_binary_ext;
using result::error_initialization_failed;
using result::error_invalid_device_address_ext;
using result::error_invalid_drm_format_modifier_plane_layout_ext;
using result::error_invalid_external_handle;
using result::error_invalid_external_handle_khr;
using result::error_invalid_opaque_capture_address;
using result::error_invalid_opaque_capture_address_khr;
using result::error_invalid_shader_nv;
using result::error_invalid_video_std_parameters_khr;
using result::error_layer_not_present;
using result::error_memory_map_failed;
using result::error_native_window_in_use_khr;
using result::error_not_permitted_ext;
using result::error_not_permitted_khr;
using result::error_out_of_date_khr;
using result::error_out_of_device_memory;
using result::error_out_of_host_memory;
using result::error_out_of_pool_memory;
using result::error_out_of_pool_memory_khr;
using result::error_pipeline_compile_required_ext;
using result::error_surface_lost_khr;
using result::error_too_many_objects;
using result::error_unknown;
using result::error_validation_failed_ext;
using result::error_video_picture_layout_not_supported_khr;
using result::error_video_profile_codec_not_supported_khr;
using result::error_video_profile_format_not_supported_khr;
using result::error_video_profile_operation_not_supported_khr;
using result::error_video_std_version_not_supported_khr;
using result::event_reset;
using result::event_set;
using result::incomplete;
using result::not_ready;
using result::operation_deferred_khr;
using result::operation_not_deferred_khr;
using result::pipeline_compile_required;
using result::pipeline_compile_required_ext;
using result::suboptimal_khr;
using result::success;
using result::thread_done_khr;
using result::thread_idle_khr;
using result::timeout;
}  // namespace hgn