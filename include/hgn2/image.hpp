#pragma once

#include <cassert>
#include <expected>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn2/device.hpp"
#include "hgn2/enums.hpp"
#include "hgn2/raii/image.hpp"

namespace hgn {
class image final {
    friend class swapchain_khr;

  public:
    image(const image&) noexcept                    = delete;
    auto operator=(const image&) noexcept -> image& = delete;
    image(image&&) noexcept                         = default;
    auto operator=(image&&) noexcept -> image&      = default;
    ~image() noexcept                               = default;

    [[nodiscard]] auto get() noexcept -> VkImage_T& { return raw_; }

  private:
    explicit image(VkImage_T& raw) noexcept : raw_{std::move(raw)} {}

    raii::image raw_;
};

class image_view final {
  public:
    image_view(const image_view&) noexcept                    = delete;
    auto operator=(const image_view&) noexcept -> image_view& = delete;
    image_view(image_view&&) noexcept                         = default;
    auto operator=(image_view&&) noexcept -> image_view&      = default;
    ~image_view() noexcept                                    = default;

    [[nodiscard]] auto get() noexcept -> VkImageView_T& { return raw_; }

    class builder final {
      public:
        auto image(image& image) noexcept -> builder& {
            image_ = image.get();
            return *this;
        }

        auto format(const format fmt) noexcept -> builder& {
            format_ = static_cast<VkFormat>(fmt);
            return *this;
        }

        [[nodiscard]] auto build(device& dev) const noexcept -> std::expected<image_view, result> {
            auto info  = VkImageViewCreateInfo{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

            info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel   = 0;
            info.subresourceRange.levelCount     = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount     = 1;

            info.viewType = VK_IMAGE_VIEW_TYPE_2D;

            if (image_) info.image = &*image_;
            info.format = format_;

            auto*      raw = static_cast<VkImageView_T*>(nullptr);
            const auto res = vkCreateImageView(&dev.get(), &info, nullptr, &raw);
            if (res != VK_SUCCESS) return make_unexpected(res);

            assert(raw != nullptr);
            return image_view{*raw, dev.get()};
        }

      private:
        std::optional<VkImage_T&> image_;
        VkFormat                  format_;
    };

  private:
    friend class image_view::builder;

    explicit image_view(VkImageView_T& raw, VkDevice_T& dev) noexcept : raw_{std::move(raw), dev} {}

    raii::image_view raw_;
};
}  // namespace hgn