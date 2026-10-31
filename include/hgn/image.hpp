#pragma once

#include <cassert>
#include <expected>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/raii/image.hpp"
#include "hgn/raw.hpp"

namespace hgn {
class image final {
    friend class swapchain_khr;

  public:
    image(const image&) noexcept                    = delete;
    auto operator=(const image&) noexcept -> image& = delete;
    image(image&&) noexcept                         = default;
    auto operator=(image&&) noexcept -> image&      = default;
    ~image() noexcept                               = default;

    [[nodiscard]] auto get() noexcept -> raw::image& { return raw_; }

  private:
    explicit image(raw::image& raw) noexcept : raw_{std::move(raw)} {}

    raii::image raw_;
};

class image_view final {
  public:
    image_view(const image_view&) noexcept                    = delete;
    auto operator=(const image_view&) noexcept -> image_view& = delete;
    image_view(image_view&&) noexcept                         = default;
    auto operator=(image_view&&) noexcept -> image_view&      = default;
    ~image_view() noexcept                                    = default;

    [[nodiscard]] auto get() noexcept -> raw::image_view& { return raw_; }

    class builder final {
        friend class device;

      public:
        auto image(image& image) noexcept -> builder& {
            image_ = image.get();
            return *this;
        }

        auto format(const format fmt) noexcept -> builder& {
            format_ = static_cast<VkFormat>(fmt);
            return *this;
        }

        [[nodiscard]] auto build() const noexcept -> std::expected<image_view, result> {
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

            auto*      raw = static_cast<raw::image_view*>(nullptr);
            const auto res = vkCreateImageView(&dev_.get(), &info, nullptr, &raw);
            if (res != VK_SUCCESS) return make_unexpected(res);

            assert(raw != nullptr);
            return image_view{*raw, dev_.get()};
        }

      private:
        explicit builder(raw::device& dev) noexcept : dev_{std::ref(dev)} {}

        std::reference_wrapper<raw::device> dev_;
        std::optional<raw::image&>          image_;
        VkFormat                            format_{};
    };

  private:
    friend class image_view::builder;

    explicit image_view(raw::image_view& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::image_view raw_;
};
}  // namespace hgn