#pragma once

#include <vulkan/vulkan.h>

namespace hgn::raw {
using instance        = VkInstance_T;
using surface_khr     = VkSurfaceKHR_T;
using physical_device = VkPhysicalDevice_T;
using device          = VkDevice_T;
using queue           = VkQueue_T;
using semaphore       = VkSemaphore_T;
using fence           = VkFence_T;

using swapchain_khr = VkSwapchainKHR_T;
using image         = VkImage_T;
using image_view    = VkImageView_T;

using command_pool   = VkCommandPool_T;
using command_buffer = VkCommandBuffer_T;

using buffer          = VkBuffer_T;
using device_memory   = VkDeviceMemory_T;
using shader_module   = VkShaderModule_T;
using pipeline_layout = VkPipelineLayout_T;
using pipeline        = VkPipeline_T;
}  // namespace hgn::raw