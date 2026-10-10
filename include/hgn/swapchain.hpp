#pragma once

#include <vulkan/vulkan.h>
#include <cassert>
#include <expected>
#include <functional>
#include <vector>

#include "hgn/device.hpp"
#include "hgn/result.hpp"
#include "hgn/surface.hpp"
#include "hgn/types.hpp"

namespace hgn {
class swapchain_create_info_khr final {
  public:
    explicit swapchain_create_info_khr() noexcept {  // NOLINT
        native_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    }

    auto surface(surface_khr& surface) noexcept -> swapchain_create_info_khr& {
        native_.surface = &surface;
        return *this;
    }

    auto min_image_count(const u32 count) noexcept -> swapchain_create_info_khr& {
        native_.minImageCount = count;
        return *this;
    }

    auto image_format(const format f) noexcept -> swapchain_create_info_khr& {
        native_.imageFormat = static_cast<VkFormat>(f);
        return *this;
    }

    auto image_color_space(const color_space_khr color) noexcept -> swapchain_create_info_khr& {
        native_.imageColorSpace = static_cast<VkColorSpaceKHR>(color);
        return *this;
    }

    auto image_extent(const extent2d extent) noexcept -> swapchain_create_info_khr& {
        native_.imageExtent = extent;
        return *this;
    }

    auto image_array_layers(const u32 layers) noexcept -> swapchain_create_info_khr& {
        native_.imageArrayLayers = layers;
        return *this;
    }

    auto image_usage(const VkImageUsageFlags usage) noexcept -> swapchain_create_info_khr& {
        native_.imageUsage = usage;
        return *this;
    }

    auto image_sharing_mode(const VkSharingMode mode) noexcept -> swapchain_create_info_khr& {
        native_.imageSharingMode = mode;
        return *this;
    }

    auto queue_family_indices(const std::span<const u32> indices) noexcept
        -> swapchain_create_info_khr& {
        native_.queueFamilyIndexCount = static_cast<u32>(indices.size());
        native_.pQueueFamilyIndices   = indices.data();
        return *this;
    }

    auto pre_transform(const surface_transform_flag_bits_khr transform) noexcept
        -> swapchain_create_info_khr& {
        native_.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(transform);
        return *this;
    }

    auto composite_alpha(const VkCompositeAlphaFlagBitsKHR bits) noexcept
        -> swapchain_create_info_khr& {
        native_.compositeAlpha = bits;
        return *this;
    }

    auto present_mode(const present_mode_khr mode) noexcept -> swapchain_create_info_khr& {
        native_.presentMode = static_cast<VkPresentModeKHR>(mode);
        return *this;
    }

    auto clipped(const bool cond) noexcept -> swapchain_create_info_khr& {
        native_.clipped = static_cast<u32>(cond);
        return *this;
    }

  private:
    VkSwapchainCreateInfoKHR native_;
};

using swapchain = VkSwapchainKHR_T;

[[nodiscard]] inline auto try_make_swapchain(device& dev, swapchain_create_info_khr& info) noexcept
    -> std::expected<ref_w<swapchain>, result> {
    auto*      chain       = static_cast<swapchain*>(nullptr);
    auto       native_info = std::bit_cast<VkSwapchainCreateInfoKHR>(info);
    const auto res         = vkCreateSwapchainKHR(&dev, &native_info, nullptr, &chain);
    if (res != VK_SUCCESS) return make_result(res);

    assert(chain != nullptr);

    return std::ref(*chain);
}

using image = VkImage_T;

[[nodiscard]] inline auto try_get_swapchain_images_khr(device& dev, swapchain& chain) noexcept
    -> std::expected<std::vector<ref_w<image>>, result> {
    auto       count = u32{};
    const auto res1  = vkGetSwapchainImagesKHR(&dev, &chain, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto images = std::vector<image*>(count);

    const auto res2 = vkGetSwapchainImagesKHR(&dev, &chain, &count, images.data());
    if (res2 != VK_SUCCESS) return make_result(res2);

    auto out = std::vector<ref_w<image>>{};
    out.reserve(count);

    for (auto* image_ptr : images) {
        assert(image_ptr != nullptr);
        out.emplace_back(std::ref(*image_ptr));
    }

    return out;
};

inline void destroy_swapchain_khr(device& dev, swapchain& target) {
    vkDestroySwapchainKHR(&dev, &target, nullptr);
}
}  // namespace hgn