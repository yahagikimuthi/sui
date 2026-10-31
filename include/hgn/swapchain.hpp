#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <limits>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/raii/image.hpp"
#include "hgn/raw.hpp"
#include "hgn/structs.hpp"
#include "hgn/surface.hpp"
#include "hgn/sync.hpp"

namespace hgn {
class swapchain_khr final {
    friend class swapchain_khr_builder;

  public:
    swapchain_khr(const swapchain_khr&) noexcept                    = delete;
    auto operator=(const swapchain_khr&) noexcept -> swapchain_khr& = delete;
    swapchain_khr(swapchain_khr&&) noexcept                         = default;
    auto operator=(swapchain_khr&&) noexcept -> swapchain_khr&      = default;
    ~swapchain_khr() noexcept                                       = default;

    [[nodiscard]] auto make_images_khr() noexcept -> std::expected<std::vector<image>, result> {
        auto       count = std::uint32_t{};
        const auto res1  = vkGetSwapchainImagesKHR(&raw_.init_device(), &get(), &count, nullptr);
        if (res1 != VK_SUCCESS) return make_unexpected(res1);

        auto       raws = std::vector<raw::image*>(count);
        const auto res2 = vkGetSwapchainImagesKHR(&raw_.init_device(), &get(), &count, raws.data());
        if (res2 != VK_SUCCESS) return make_unexpected(res2);

        return raws | std::views::transform([](raw::image* raw) noexcept -> image {
                   assert(raw != nullptr);
                   return image{*raw};
               }) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto acquire_next_image(semaphore& semapho) noexcept
        -> std::expected<std::uint32_t, result> {
        auto       index = std::uint32_t{};
        const auto res   = vkAcquireNextImageKHR(
            &raw_.init_device(),
            &get(),
            std::numeric_limits<std::uint32_t>::max(),
            &semapho.get(),
            nullptr,
            &index
        );
        if (res != VK_SUCCESS) return make_unexpected(res);

        return index;
    }

    [[nodiscard]] auto image_format() const noexcept -> format { return image_format_; }

    [[nodiscard]] auto get() noexcept -> VkSwapchainKHR_T& { return raw_; }

  private:
    explicit swapchain_khr(VkSwapchainKHR_T& raw, raw::device& dev, const format fmt) noexcept
        : raw_{std::move(raw), dev}, image_format_{fmt} {}

    raii::swapchain_khr raw_;
    format              image_format_;
};

class swapchain_khr_builder final {
    friend class device;

  public:
    auto surface(surface_khr& surface) noexcept -> swapchain_khr_builder& {
        info_.surface = &surface.get();
        return *this;
    }

    auto min_image_count(const std::uint32_t count) noexcept -> swapchain_khr_builder& {
        info_.minImageCount = count;
        return *this;
    }

    auto image_format(const format fmt) noexcept -> swapchain_khr_builder& {
        info_.imageFormat = static_cast<VkFormat>(fmt);
        return *this;
    }

    auto image_color_space(const color_space_khr space) noexcept -> swapchain_khr_builder& {
        info_.imageColorSpace = static_cast<VkColorSpaceKHR>(space);
        return *this;
    }

    auto image_extent(const extent2d extent) noexcept -> swapchain_khr_builder& {
        info_.imageExtent = std::bit_cast<VkExtent2D>(extent);
        return *this;
    }

    auto pre_transform(const surface_transform_flag_bits_khr transform) noexcept
        -> swapchain_khr_builder& {
        info_.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(transform);
        return *this;
    }

    auto present_mode(const present_mode_khr mode) noexcept -> swapchain_khr_builder& {
        info_.presentMode = static_cast<VkPresentModeKHR>(mode);
        return *this;
    }

    auto queue_family_indices(const std::span<const std::uint32_t> indices) noexcept
        -> swapchain_khr_builder& {
        queue_family_indices_.assign_range(indices);
        return *this;
    }

    auto image_sharing_mode(const sharing_mode mode) noexcept -> swapchain_khr_builder& {
        info_.imageSharingMode = static_cast<VkSharingMode>(mode);
        return *this;
    }

    [[nodiscard]] auto build() noexcept -> std::expected<swapchain_khr, result> {
        info_.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_family_indices_.size());
        info_.pQueueFamilyIndices   = queue_family_indices_.data();

        auto*      raw = static_cast<VkSwapchainKHR_T*>(nullptr);
        const auto res = vkCreateSwapchainKHR(&dev_.get(), &info_, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return swapchain_khr{*raw, dev_.get(), static_cast<format>(info_.imageFormat)};
    }

  private:
    explicit swapchain_khr_builder(raw::device& dev) noexcept : dev_{std::ref(dev)} {
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
}  // namespace hgn