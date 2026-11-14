#pragma once

#include <cstddef>
#include <expected>
#include <vector>

#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <hgn/swapchain.hpp>

#include "sui/detail/context/hardware.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class display final {
  public:
    [[nodiscard]] static auto make(hardware& hard) noexcept -> std::expected<display, error> {
        auto swapchain =
            hard.device()
                .get_swapchain_creator(hard.surface())
                .extent(800, 600)
                .surface_format(hgn::format::b8g8r8a8_srgb, hgn::color_space_khr::srgb_nonlinear)
                .present_mode(hgn::present_mode_khr::mailbox)
                .queue_families(hard.graphics_family(), hard.present_family())
                .create();
        if (not swapchain) return make_error(runtime_error, "Failed to make swapchain.");

        auto images = swapchain->get_images();
        if (not images) return make_error(runtime_error, "Failed to get images.");

        auto views = std::vector<hgn::image_view>{};
        views.reserve(images->size());

        for (auto& image : *images) {
            auto view = hard.device()
                            .get_image_view_creator()
                            .image(image)
                            .format(swapchain->image_format())
                            .create();
            if (not view) return make_error(runtime_error, "Failed to make image view.");

            views.emplace_back(std::move(*view));
        }

        return display{std::move(*swapchain), std::move(*images), std::move(views)};
    }

    [[nodiscard]] auto get_image(const std::size_t i) noexcept -> hgn::borrowed_image& {
        return images_[i];
    }
    [[nodiscard]] auto get_view(const std::size_t i) noexcept -> hgn::image_view& {
        return views_[i];
    }

    [[nodiscard]] auto swapchain() noexcept -> hgn::swapchain_khr& { return swapchain_; }

  private:
    display(
        hgn::swapchain_khr&&               swapchain,
        std::vector<hgn::borrowed_image>&& images,
        std::vector<hgn::image_view>&&     views
    ) noexcept
        : swapchain_{std::move(swapchain)}, images_{std::move(images)}, views_{std::move(views)} {}

    hgn::swapchain_khr               swapchain_;
    std::vector<hgn::borrowed_image> images_;
    std::vector<hgn::image_view>     views_;
};
}  // namespace sui::detail