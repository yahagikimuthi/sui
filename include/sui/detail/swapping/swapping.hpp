#pragma once

#include <cstddef>
#include <expected>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <hgn/swapchain.hpp>

#include "sui/detail/hardware/hardware.hpp"
#include "sui/detail/swapping/images.hpp"
#include "sui/detail/swapping/swapchain.hpp"

namespace sui {
class swapping final {
  public:
    [[nodiscard]] static auto try_make(hardware& hard) noexcept -> std::expected<swapping, error> {
        auto chain = swapchain::try_make(hard);
        if (not chain) return make_error(runtime_error, "Failed to make swapchain.", chain.error());

        auto images = image_container::try_make(hard.get_device(), *chain);
        if (not images)
            return make_error(runtime_error, "Failed to make image container.", images.error());

        auto views =
            image_view_container::try_make(*images, hard.get_device(), chain->setting_format());
        if (not views)
            return make_error(runtime_error, "Failed to make image view container.", views.error());

        return swapping{std::move(*chain), std::move(*images), std::move(*views)};
    }

    swapping(const swapping&) noexcept                    = delete;
    auto operator=(const swapping&) noexcept -> swapping& = delete;
    swapping(swapping&&) noexcept                         = default;
    auto operator=(swapping&&) noexcept -> swapping&      = default;
    ~swapping() noexcept                                  = default;

    [[nodiscard]] auto setting_format() const noexcept -> hgn::format {
        return chain_.setting_format();
    }

    [[nodiscard]] auto native_swapchain() noexcept -> hgn::swapchain_khr& { return chain_.get(); }

    [[nodiscard]] auto native_image(const std::size_t image_index) noexcept -> hgn::image& {
        return images_[image_index];
    }

    [[nodiscard]] auto native_image_view(const std::size_t image_index) noexcept
        -> hgn::image_view& {
        return image_views_[image_index];
    }

  private:
    explicit swapping(
        swapchain&& chain, image_container&& images, image_view_container&& views
    ) noexcept
        : chain_{std::move(chain)}, images_{std::move(images)}, image_views_{std::move(views)} {}

    swapchain            chain_;
    image_container      images_;
    image_view_container image_views_;
};
}  // namespace sui