#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "hgn2/device.hpp"
#include "hgn2/enums.hpp"

namespace hgn {
class swapchain_khr final {
  public:
    swapchain_khr(const swapchain_khr&) noexcept                    = delete;
    auto operator=(const swapchain_khr&) noexcept -> swapchain_khr& = delete;

    swapchain_khr(swapchain_khr&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(swapchain_khr&& other) noexcept -> swapchain_khr& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~swapchain_khr() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkSwapchainKHR_T& { return *raw_; }

    class builder final {
      public:
        [[nodiscard]] auto build(device& dev) noexcept -> std::expected<swapchain_khr, result> {
            auto info             = VkSwapchainCreateInfoKHR{};  // NOLINT
            info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            info.imageArrayLayers = 1;
            info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            info.clipped          = VK_TRUE;

            if (surface_) info.surface = &*surface_;
            info.minImageCount         = min_image_count_;
            info.imageFormat           = format_;
            info.imageColorSpace       = color_space_;
            info.imageExtent           = image_extent_;
            info.imageSharingMode      = image_sharing_mode_;
            info.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_family_indices_.size());
            info.pQueueFamilyIndices   = queue_family_indices_.data();
            info.preTransform          = pre_transform_;
            info.presentMode           = present_mode_;

            auto*      raw = static_cast<VkSwapchainKHR_T*>(nullptr);
            const auto res = vkCreateSwapchainKHR(&dev.get(), &info, nullptr, &raw);
            if (res != VK_SUCCESS) return make_unexpected(res);

            assert(raw != nullptr);
            return swapchain_khr{*raw, dev.get()};
        }

      private:
        std::optional<VkSurfaceKHR_T&> surface_;
        std::uint32_t                  min_image_count_{};
        VkFormat                       format_{};
        VkColorSpaceKHR                color_space_{};
        VkExtent2D                     image_extent_{};
        VkSharingMode                  image_sharing_mode_{};
        std::vector<std::uint32_t>     queue_family_indices_;
        VkSurfaceTransformFlagBitsKHR pre_transform_{static_cast<VkSurfaceTransformFlagBitsKHR>(0)};
        VkPresentModeKHR              present_mode_;
    };

  private:
    friend swapchain_khr::builder;

    explicit swapchain_khr(VkSwapchainKHR_T& raw, VkDevice_T& dev) noexcept
        : raw_{raw}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (raw_) vkDestroySwapchainKHR(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkSwapchainKHR_T&>   raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};
}  // namespace hgn