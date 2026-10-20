#pragma once

#include <cstddef>
#include <utility>

#include <hgn/image.hpp>

#include "sui/hardware.hpp"
#include "sui/images.hpp"
#include "sui/swapchain.hpp"

namespace sui {
class render_context final {
  public:
    [[nodiscard]] static auto try_make(hardware& hard) noexcept
        -> std::expected<render_context, error> {
        auto chain = swapchain::try_make(
            hard.get_physical_device(), hard.native_surface(), hard.native_device()
        );
        if (not chain) return make_error(runtime_error, "Failed to make swapchain.", chain.error());

        auto images = image_container::try_make(hard.native_device(), chain->get());
        if (not images)
            return make_error(runtime_error, "Failed to make image container.", images.error());

        auto views =
            image_view_container::try_make(*images, hard.native_device(), chain->setting_format());
        if (not views)
            return make_error(runtime_error, "Failed to make image view container.", views.error());

        return render_context{std::move(*chain), std::move(*images), std::move(*views)};
    }

    [[nodiscard]] auto native_swapchain() noexcept -> hgn::swapchain& { return chain_.get(); }

    [[nodiscard]] auto native_image(const std::size_t image_index) noexcept -> hgn::image& {
        return images_[image_index];
    }

    [[nodiscard]] auto native_image_view(const std::size_t image_index) noexcept
        -> hgn::image_view& {
        return image_views_[image_index];
    }

  private:
    explicit render_context(
        swapchain&& chain, image_container&& images, image_view_container&& views
    ) noexcept
        : chain_{std::move(chain)}, images_{std::move(images)}, image_views_{std::move(views)} {}

    swapchain            chain_;
    image_container      images_;
    image_view_container image_views_;
};
}  // namespace sui