#pragma once

#include <vulkan/vulkan.hpp>

#include "hgn/instance.hpp"

namespace hgn {
using surface_khr = VkSurfaceKHR_T;

inline void destroy_surface_khr(instance& ins, surface_khr& target) noexcept {
    vkDestroySurfaceKHR(&ins, &target, nullptr);
}
}  // namespace hgn