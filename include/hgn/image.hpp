#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <limits>
#include <vector>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/swapchain.hpp"
#include "hgn/sync.hpp"

namespace hgn {
using image = VkImage_T;

[[nodiscard]] inline auto try_get_swapchain_images_khr(device& dev, swapchain& chain) noexcept
    -> std::expected<std::vector<std::reference_wrapper<image>>, result> {
    auto       count = std::uint32_t{};
    const auto res1  = vkGetSwapchainImagesKHR(&dev, &chain, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto images = std::vector<image*>(count);

    const auto res2 = vkGetSwapchainImagesKHR(&dev, &chain, &count, images.data());
    if (res2 != VK_SUCCESS) return make_result(res2);

    auto out = std::vector<std::reference_wrapper<image>>{};
    out.reserve(count);
    out.assign_range(
        images | std::views::transform([](image* ptr) noexcept -> std::reference_wrapper<image> {
            assert(ptr != nullptr);
            return std::ref(*ptr);
        })
    );

    return out;
};

class image_view_create_info final {
  public:
    explicit image_view_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        native_.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        native_.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        native_.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        native_.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        native_.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        native_.subresourceRange.baseMipLevel   = 0;
        native_.subresourceRange.levelCount     = 1;
        native_.subresourceRange.baseArrayLayer = 0;
        native_.subresourceRange.layerCount     = 1;
    }

    auto setting_image(image& image_ref) noexcept -> image_view_create_info& {
        native_.image = &image_ref;
        return *this;
    }

    auto view_type(const image_view_type type) noexcept -> image_view_create_info& {
        native_.viewType = static_cast<VkImageViewType>(type);
        return *this;
    }

    auto format(const format f) noexcept -> image_view_create_info& {
        native_.format = static_cast<VkFormat>(f);
        return *this;
    }

  private:
    VkImageViewCreateInfo native_{};
};

using image_view = VkImageView_T;

[[nodiscard]] inline auto try_make_image_view(
    device& dev, const image_view_create_info& info
) noexcept -> std::expected<std::reference_wrapper<image_view>, result> {
    auto*      view        = static_cast<image_view*>(nullptr);
    auto       native_info = std::bit_cast<VkImageViewCreateInfo>(info);
    const auto res         = vkCreateImageView(&dev, &native_info, nullptr, &view);
    if (res != VK_SUCCESS) return make_result(res);

    assert(view != nullptr);
    return std::ref(*view);
}

inline void destroy_image_view(device& dev, image_view& view) noexcept {
    vkDestroyImageView(&dev, &view, nullptr);
}

[[nodiscard]] inline auto try_acquire_next_image_khr(
    device&             dev,
    swapchain&          chain,
    semaphore&          semaph,
    fence&              fence_ref,
    const std::uint32_t timeout = std::numeric_limits<std::uint32_t>::max()
) noexcept -> std::expected<std::uint32_t, result> {
    auto       index = std::uint32_t{};
    const auto res   = vkAcquireNextImageKHR(&dev, &chain, timeout, &semaph, &fence_ref, &index);
    if (res != VK_SUCCESS) return make_result(res);
    return index;
}

[[nodiscard]] inline auto try_acquire_next_image_khr(
    device&             dev,
    swapchain&          chain,
    semaphore&          semaph,
    const std::uint32_t timeout = std::numeric_limits<std::uint32_t>::max()
) noexcept -> std::expected<std::uint32_t, result> {
    auto       index = std::uint32_t{};
    const auto res   = vkAcquireNextImageKHR(&dev, &chain, timeout, &semaph, nullptr, &index);
    if (res != VK_SUCCESS) return make_result(res);
    return index;
}
}  // namespace hgn