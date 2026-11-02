#pragma once

#include <bit>
#include <cstdint>
#include <functional>
#include <ranges>
#include <span>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/queue_family.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/structs.hpp"
#include "hgn/surface.hpp"
#include "hgn/swapchain.hpp"

namespace hgn {
class swapchain_khr_creator final {
    friend class device;

  public:
    auto surface(surface_khr& surface) noexcept -> swapchain_khr_creator& {
        info_.surface = &surface.get();
        return *this;
    }

    auto min_image_count(const std::uint32_t count) noexcept -> swapchain_khr_creator& {
        info_.minImageCount = count;
        return *this;
    }

    auto image_format(const format fmt) noexcept -> swapchain_khr_creator& {
        info_.imageFormat = static_cast<VkFormat>(fmt);
        return *this;
    }

    auto image_color_space(const color_space_khr space) noexcept -> swapchain_khr_creator& {
        info_.imageColorSpace = static_cast<VkColorSpaceKHR>(space);
        return *this;
    }

    auto image_extent(const extent2d extent) noexcept -> swapchain_khr_creator& {
        info_.imageExtent = std::bit_cast<VkExtent2D>(extent);
        return *this;
    }

    auto pre_transform(const surface_transform_flag_bits_khr transform) noexcept
        -> swapchain_khr_creator& {
        info_.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(transform);
        return *this;
    }

    auto present_mode(const present_mode_khr mode) noexcept -> swapchain_khr_creator& {
        info_.presentMode = static_cast<VkPresentModeKHR>(mode);
        return *this;
    }

    auto queue_families(const std::span<const queue_family> families) noexcept
        -> swapchain_khr_creator& {
        queue_family_indices_.assign_range(families | std::views::transform(&queue_family::index));
        return *this;
    }

    auto image_sharing_mode(const sharing_mode mode) noexcept -> swapchain_khr_creator& {
        info_.imageSharingMode = static_cast<VkSharingMode>(mode);
        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::expected<swapchain_khr, result> {
        info_.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_family_indices_.size());
        info_.pQueueFamilyIndices   = queue_family_indices_.data();

        auto*      raw = static_cast<raw::swapchain_khr*>(nullptr);
        const auto res = vkCreateSwapchainKHR(&dev_.get(), &info_, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return swapchain_khr{*raw, dev_.get(), static_cast<format>(info_.imageFormat)};
    }

  private:
    explicit swapchain_khr_creator(raw::device& dev) noexcept : dev_{std::ref(dev)} {
        info_.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info_.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info_.imageArrayLayers = 1;
        info_.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info_.clipped          = VK_TRUE;
    }

    VkSwapchainCreateInfoKHR            info_{};  // NOLINT
    std::vector<std::uint32_t>          queue_family_indices_;
    std::reference_wrapper<raw::device> dev_;
};

class image_view_creator final {
    friend class device;

  public:
    auto image(image& image) noexcept -> image_view_creator& {
        info_.image = &image.get();
        return *this;
    }

    auto format(const format fmt) noexcept -> image_view_creator& {
        info_.format = static_cast<VkFormat>(fmt);
        return *this;
    }

    [[nodiscard]] auto create() const noexcept -> std::expected<image_view, result> {
        auto*      raw = static_cast<raw::image_view*>(nullptr);
        const auto res = vkCreateImageView(&dev_.get(), &info_, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return image_view{*raw, dev_.get()};
    }

  private:
    explicit image_view_creator(raw::device& dev) noexcept : dev_{std::ref(dev)} {
        info_.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        info_.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        info_.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        info_.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        info_.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        info_.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        info_.subresourceRange.baseMipLevel   = 0;
        info_.subresourceRange.levelCount     = 1;
        info_.subresourceRange.baseArrayLayer = 0;
        info_.subresourceRange.layerCount     = 1;

        info_.viewType = VK_IMAGE_VIEW_TYPE_2D;
    }

    VkImageViewCreateInfo               info_{};
    std::reference_wrapper<raw::device> dev_;
};
}  // namespace hgn