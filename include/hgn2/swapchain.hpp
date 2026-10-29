#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn2/device.hpp"
#include "hgn2/enums.hpp"
#include "hgn2/image.hpp"
#include "hgn2/raii/image.hpp"
#include "hgn2/structs.hpp"
#include "hgn2/surface.hpp"

namespace hgn {
class swapchain_khr final {
  public:
    swapchain_khr(const swapchain_khr&) noexcept                    = delete;
    auto operator=(const swapchain_khr&) noexcept -> swapchain_khr& = delete;
    swapchain_khr(swapchain_khr&&) noexcept                         = default;
    auto operator=(swapchain_khr&&) noexcept -> swapchain_khr&      = default;
    ~swapchain_khr() noexcept                                       = default;

    [[nodiscard]] auto get_images_khr(device& dev) noexcept
        -> std::expected<std::vector<image>, result> {
        auto       count = std::uint32_t{};
        const auto res1  = vkGetSwapchainImagesKHR(&dev.get(), &get(), &count, nullptr);
        if (res1 != VK_SUCCESS) return make_unexpected(res1);

        auto       raws = std::vector<VkImage_T*>(count);
        const auto res2 = vkGetSwapchainImagesKHR(&dev.get(), &get(), &count, raws.data());
        if (res2 != VK_SUCCESS) return make_unexpected(res2);

        return raws | std::views::transform([](VkImage_T* raw) noexcept -> image {
                   assert(raw != nullptr);
                   return image{*raw};
               }) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto image_format() const noexcept -> format { return image_format_; }

    [[nodiscard]] auto get() noexcept -> VkSwapchainKHR_T& { return raw_; }

    class builder final {
      public:
        auto surface(surface_khr& surface) noexcept -> builder& {
            surface_ = surface.get();
            return *this;
        }

        auto min_image_count(const std::uint32_t count) noexcept -> builder& {
            min_image_count_ = count;
            return *this;
        }

        auto image_format(const format fmt) noexcept -> builder& {
            format_ = static_cast<VkFormat>(fmt);
            return *this;
        }

        auto image_color_space(const color_space_khr space) noexcept -> builder& {
            color_space_ = static_cast<VkColorSpaceKHR>(space);
            return *this;
        }

        auto image_extent(const extent2d extent) noexcept -> builder& {
            image_extent_ = std::bit_cast<VkExtent2D>(extent);
            return *this;
        }

        auto pre_transform(const surface_transform_flag_bits_khr transform) noexcept -> builder& {
            pre_transform_ = static_cast<VkSurfaceTransformFlagBitsKHR>(transform);
            return *this;
        }

        auto present_mode(const present_mode_khr mode) noexcept -> builder& {
            present_mode_ = static_cast<VkPresentModeKHR>(mode);
            return *this;
        }

        auto queue_family_indices(const std::span<const std::uint32_t> indices) noexcept
            -> builder& {
            queue_family_indices_.assign_range(indices);
            return *this;
        }

        auto image_sharing_mode(const sharing_mode mode) noexcept -> builder& {
            image_sharing_mode_ = static_cast<VkSharingMode>(mode);
            return *this;
        }

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
            return swapchain_khr{*raw, dev.get(), static_cast<format>(format_)};
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

    explicit swapchain_khr(VkSwapchainKHR_T& raw, VkDevice_T& dev, const format fmt) noexcept
        : raw_{std::move(raw), dev}, image_format_{fmt} {}

    raii::swapchain_khr raw_;
    format              image_format_;
};
}  // namespace hgn