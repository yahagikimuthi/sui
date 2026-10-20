#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <span>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/surface.hpp"

namespace hgn {
class swapchain_create_info_khr final {
  public:
    explicit swapchain_create_info_khr() noexcept = default;

    swapchain_create_info_khr(const swapchain_create_info_khr& other) noexcept
        : native_{other.native_}, queue_family_indices_{other.queue_family_indices_} {
        reset();
    }

    auto operator=(const swapchain_create_info_khr& other) noexcept -> swapchain_create_info_khr& {
        if (this == &other) return *this;

        native_               = other.native_;
        queue_family_indices_ = other.queue_family_indices_;

        reset();
        return *this;
    }

    swapchain_create_info_khr(swapchain_create_info_khr&& other) noexcept
        : native_{other.native_}, queue_family_indices_{std::move(other.queue_family_indices_)} {
        reset();
    }

    auto operator=(swapchain_create_info_khr&& other) noexcept -> swapchain_create_info_khr& {
        if (this == &other) return *this;

        native_               = other.native_;
        queue_family_indices_ = std::move(other.queue_family_indices_);

        reset();
        return *this;
    }

    ~swapchain_create_info_khr() noexcept = default;

    auto surface(surface_khr& surface) noexcept -> swapchain_create_info_khr& {
        native_.surface = &surface;
        return *this;
    }

    auto min_image_count(const std::uint32_t count) noexcept -> swapchain_create_info_khr& {
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

    auto image_array_layers(const std::uint32_t layers) noexcept -> swapchain_create_info_khr& {
        native_.imageArrayLayers = layers;
        return *this;
    }

    auto image_usage(const image_usage_flags usage) noexcept -> swapchain_create_info_khr& {
        native_.imageUsage = static_cast<VkImageUsageFlags>(usage);
        return *this;
    }

    auto image_sharing_mode(const sharing_mode mode) noexcept -> swapchain_create_info_khr& {
        native_.imageSharingMode = static_cast<VkSharingMode>(mode);
        return *this;
    }

    auto queue_family_indices(const std::span<const std::uint32_t> indices) noexcept
        -> swapchain_create_info_khr& {
        queue_family_indices_.assign_range(indices);
        native_.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_family_indices_.size());
        native_.pQueueFamilyIndices   = queue_family_indices_.data();
        return *this;
    }

    auto pre_transform(const surface_transform_flag_bits_khr transform) noexcept
        -> swapchain_create_info_khr& {
        native_.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(transform);
        return *this;
    }

    auto composite_alpha(const composite_alpha_flag_bits_khr bits) noexcept
        -> swapchain_create_info_khr& {
        native_.compositeAlpha = static_cast<VkCompositeAlphaFlagBitsKHR>(bits);
        return *this;
    }

    auto present_mode(const present_mode_khr mode) noexcept -> swapchain_create_info_khr& {
        native_.presentMode = static_cast<VkPresentModeKHR>(mode);
        return *this;
    }

    auto clipped(const bool cond) noexcept -> swapchain_create_info_khr& {
        native_.clipped = static_cast<std::uint32_t>(cond);
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkSwapchainCreateInfoKHR& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pQueueFamilyIndices != nullptr)
            native_.pQueueFamilyIndices = queue_family_indices_.data();
    }

    VkSwapchainCreateInfoKHR native_{
        .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext                 = nullptr,
        .flags                 = 0,
        .surface               = nullptr,
        .minImageCount         = 0,
        .imageFormat           = static_cast<VkFormat>(0),
        .imageColorSpace       = static_cast<VkColorSpaceKHR>(0),
        .imageExtent           = {},
        .imageArrayLayers      = 0,
        .imageUsage            = 0,
        .imageSharingMode      = static_cast<VkSharingMode>(0),
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
        .preTransform          = static_cast<VkSurfaceTransformFlagBitsKHR>(0),
        .compositeAlpha        = static_cast<VkCompositeAlphaFlagBitsKHR>(0),
        .presentMode           = static_cast<VkPresentModeKHR>(0),
        .clipped               = 0,
        .oldSwapchain          = nullptr
    };
    std::vector<std::uint32_t> queue_family_indices_;
};

using swapchain = VkSwapchainKHR_T;

[[nodiscard]] inline auto try_make_swapchain(
    device& dev, const swapchain_create_info_khr& info
) noexcept -> std::expected<std::reference_wrapper<swapchain>, result> {
    auto*      chain = static_cast<swapchain*>(nullptr);
    const auto res   = vkCreateSwapchainKHR(&dev, &info.get(), nullptr, &chain);
    if (res != VK_SUCCESS) return make_result(res);

    assert(chain != nullptr);

    return std::ref(*chain);
}

inline void destroy_swapchain_khr(device& dev, swapchain& target) {
    vkDestroySwapchainKHR(&dev, &target, nullptr);
}
}  // namespace hgn