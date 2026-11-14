#pragma once

#include <expected>
#include <utility>
#include <vector>

#include <akr/akr.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <hgn/surface.hpp>
#include <hgn/swapchain.hpp>

#include "sui/hardware.hpp"

namespace sui {
class display final {
  public:
    [[nodiscard]] static auto create(hardware& hard
    ) noexcept -> std::expected<display, akr::error> {
        auto& dev     = hard.device();
        auto& surface = hard.surface();

        auto swapchain =
            dev.get_swapchain_creator(surface)
                .extent(800, 600)
                .present_mode(hgn::present_mode_khr::mailbox)
                .surface_format(hgn::format::b8g8r8a8_srgb, hgn::color_space_khr::srgb_nonlinear)
                .queue_families(hard.graphics_queue().family(), hard.present_queue().family())
                .create();
        if (not swapchain)
            return akr::make_error(akr::runtime_error, "Failed to create swapchain.");

        auto images = swapchain->get_images();
        if (not images)
            return akr::make_error(akr::runtime_error, "Failed to get images from swapchain.");

        auto views = std::vector<hgn::image_view>{};
        for (auto& image : *images) {
            auto view = dev.get_image_view_creator()
                            .image(image)
                            .format(swapchain->image_format())
                            .create();
            if (not view)
                return akr::make_error(akr::runtime_error, "Failed to create image view.");

            views.emplace_back(std::move(*view));
        }

        return display{std::move(*swapchain), *images, std::move(views)};
    }

  private:
    display(
        hgn::swapchain_khr               swapchain,
        std::vector<hgn::borrowed_image> images,
        std::vector<hgn::image_view>     views
    ) noexcept
        : swapchain_{std::move(swapchain)}, images_{std::move(images)}, views_{std::move(views)} {}

    hgn::swapchain_khr               swapchain_;
    std::vector<hgn::borrowed_image> images_;
    std::vector<hgn::image_view>     views_;
};
}  // namespace sui