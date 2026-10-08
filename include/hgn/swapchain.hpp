#pragma once

#include <vulkan/vulkan.h>

#include "hgn/device.hpp"

namespace hgn {
using swapchain = VkSwapchainKHR_T;

inline void destroy_swapchain_khr(device& dev, swapchain& target) {
    vkDestroySwapchainKHR(&dev, &target, nullptr);
}
}  // namespace hgn