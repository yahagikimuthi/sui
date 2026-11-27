#pragma once

#include <cstddef>
#include <expected>
#include <vector>

#include <hgn/allocator.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>

#include "wk/detail/detail.hpp"
#include "wk/extent.hpp"

namespace wk::detail {
class depth_buffer_manager final {
  public:
    depth_buffer_manager() noexcept = default;

    [[nodiscard]] auto push() noexcept;

    [[nodiscard]] auto get(const std::size_t i) noexcept -> image& { return images_[i]; }

    [[nodiscard]] auto recreate_all(
        hgn::allocator& allocator, hgn::device& dev, const extentu32 extent
    ) noexcept -> std::expected<void, hgn::result> {
        for (auto& img : images_) {
            if (auto res = recreate(allocator, dev, extent, img); not res) {
                return std::unexpected{res.error()};
            }
        }

        return {};
    }

  private:
    [[nodiscard]] static auto recreate(
        hgn::allocator& allocator, hgn::device& dev, const extentu32 extent, image& recreation
    ) noexcept -> std::expected<void, hgn::result> {
        auto img = allocator.get_image_creator()
                       .extent(extent.width, extent.height)
                       .format(recreation.img.format())
                       .usage(recreation.img.usage())
                       .create();
        if (not img) return std::unexpected{img.error()};

        auto view = dev.get_image_view_creator()
                        .image(*img)
                        .aspect_mask(recreation.view.aspect_mask())
                        .format(recreation.view.format())
                        .create();

        recreation = {.img = std::move(*img), .view = std::move(*view)};

        return {};
    }

    std::vector<image> images_;
};
}  // namespace wk::detail