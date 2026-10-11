#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <expected>
#include <functional>
#include <optional>
#include <vector>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/swapchain.hpp"
#include "hgn/types.hpp"

namespace hgn {
using image = VkImage_T;

[[nodiscard]] inline auto try_get_swapchain_images_khr(device& dev, swapchain& chain) noexcept
    -> std::expected<std::vector<std::reference_wrapper<image>>, result> {
    auto       count = u32{};
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

    image_view_create_info(const image_view_create_info& other)
        : native_{other.native_}, image_{other.image_} {
        reset();
    }

    auto operator=(const image_view_create_info& other) noexcept -> image_view_create_info& {
        if (this == &other) return *this;

        native_ = other.native_;
        image_  = other.image_;

        reset();
        return *this;
    }

    image_view_create_info(image_view_create_info&& other) noexcept
        : native_{other.native_}, image_{other.image_} {
        reset();
    }

    auto operator=(image_view_create_info&& other) noexcept -> image_view_create_info& {
        if (this == &other) return *this;

        native_ = other.native_;
        image_  = other.image_;

        reset();
        return *this;
    }

    ~image_view_create_info() noexcept = default;

    auto setting_image(image& image_ref) noexcept -> image_view_create_info& {
        image_        = image_ref;
        native_.image = &*image_;
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

    [[nodiscard]] auto native() const noexcept -> const VkImageViewCreateInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.image != nullptr) {
            assert(image_);
            native_.image = &*image_;
        }
    }

    VkImageViewCreateInfo native_{};
    std::optional<image&> image_;
};

using image_view = VkImageView_T;

[[nodiscard]] inline auto try_make_image_view(
    device& dev, const image_view_create_info& info
) noexcept -> std::expected<std::reference_wrapper<image_view>, result> {
    auto*      view = static_cast<image_view*>(nullptr);
    const auto res  = vkCreateImageView(&dev, &info.native(), nullptr, &view);
    if (res != VK_SUCCESS) return make_result(res);

    assert(view != nullptr);
    return std::ref(*view);
}

inline void destroy_image_view(device& dev, image_view& view) noexcept {
    vkDestroyImageView(&dev, &view, nullptr);
}
}  // namespace hgn