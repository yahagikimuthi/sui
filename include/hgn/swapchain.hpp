#pragma once

#include <vulkan/vulkan.h>
#include <cassert>
#include <expected>

#include "hgn/device.hpp"
#include "hgn/result.hpp"
#include "hgn/surface.hpp"
#include "hgn/types.hpp"

namespace hgn {
using swapchain_create_info_khr = VkSwapchainCreateInfoKHR;

class swapchain_create_info_khr_setter final {
  public:
    explicit swapchain_create_info_khr_setter(swapchain_create_info_khr& chain) noexcept
        : chain_{chain} {
        chain_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    }

    auto surface(surface_khr& surface) noexcept -> swapchain_create_info_khr_setter& {
        chain_.surface = &surface;
        return *this;
    }

    auto min_image_count(const u32 count) noexcept -> swapchain_create_info_khr_setter& {
        chain_.minImageCount = count;
        return *this;
    }

    auto image_format(const VkFormat format) noexcept -> swapchain_create_info_khr_setter& {
        chain_.imageFormat = format;
        return *this;
    }

    auto image_color_space(const VkColorSpaceKHR color) noexcept
        -> swapchain_create_info_khr_setter& {
        chain_.imageColorSpace = color;
        return *this;
    }

    auto image_extent(const extent2d extent) noexcept -> swapchain_create_info_khr_setter& {
        chain_.imageExtent = extent;
        return *this;
    }

    auto image_array_layers(const u32 layers) noexcept -> swapchain_create_info_khr_setter& {
        chain_.imageArrayLayers = layers;
        return *this;
    }

    auto image_usage(const VkImageUsageFlags usage) noexcept -> swapchain_create_info_khr_setter& {
        chain_.imageUsage = usage;
        return *this;
    }

    auto image_sharing_mode(const VkSharingMode mode) noexcept
        -> swapchain_create_info_khr_setter& {
        chain_.imageSharingMode = mode;
        return *this;
    }

    auto queue_family_indices(const std::span<const u32> indices) noexcept
        -> swapchain_create_info_khr_setter& {
        chain_.queueFamilyIndexCount = static_cast<u32>(indices.size());
        chain_.pQueueFamilyIndices   = indices.data();
        return *this;
    }

    auto pre_transform(const VkSurfaceTransformFlagBitsKHR transform) noexcept
        -> swapchain_create_info_khr_setter& {
        chain_.preTransform = transform;
        return *this;
    }

    auto composite_alpha(const VkCompositeAlphaFlagBitsKHR bits) noexcept
        -> swapchain_create_info_khr_setter& {
        chain_.compositeAlpha = bits;
        return *this;
    }

    auto present_mode(const VkPresentModeKHR mode) noexcept -> swapchain_create_info_khr_setter& {
        chain_.presentMode = mode;
        return *this;
    }

    auto clipped(const bool cond) noexcept -> swapchain_create_info_khr_setter& {
        chain_.clipped = static_cast<u32>(cond);
        return *this;
    }

  private:
    swapchain_create_info_khr& chain_;
};

using swapchain = VkSwapchainKHR_T;

[[nodiscard]] inline auto try_make_swapchain(device& dev, swapchain_create_info_khr& info) noexcept
    -> std::expected<ref_w<swapchain>, result> {
    auto*      chain = static_cast<swapchain*>(nullptr);
    const auto res   = vkCreateSwapchainKHR(&dev, &info, nullptr, &chain);
    if (res != VK_SUCCESS) return make_result(res);

    assert(chain != nullptr);

    return std::ref(*chain);
}

inline void destroy_swapchain_khr(device& dev, swapchain& target) {
    vkDestroySwapchainKHR(&dev, &target, nullptr);
}
}  // namespace hgn