#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <limits>
#include <ranges>
#include <vector>

#include <hgn2/enums.hpp>
#include <hgn2/image.hpp>
#include <hgn2/structs.hpp>
#include <hgn2/swapchain.hpp>

#include "sui2/error.hpp"
#include "sui2/hardware.hpp"

namespace sui {
class image final {
  public:
    [[nodiscard]] static auto make(hardware& hard) noexcept -> std::expected<image, error> {
        auto& physical = hard.physical();
        auto& surface  = hard.surface();

        const auto capabilities = physical.get_surface_capabilities_khr(surface);
        if (not capabilities)
            return make_error(runtime_error, "Failed to get surface capabilities.");

        const auto formats = physical.get_surface_formats_khr(surface);
        if (not formats or formats->empty())
            return make_error(runtime_error, "Failed to get surface formats.");

        const auto present_modes = physical.get_surface_present_modes_khr(surface);
        if (not present_modes)
            return make_error(runtime_error, "Failed to get surface present modes.");

        auto aligned_formats =
            *formats |
            std::views::filter([](const hgn::surface_format_khr format) noexcept -> bool {
                return format.format() == hgn::format::b8g8r8a8_srgb and
                       format.color_space() == hgn::color_space_khr::srgb_nonlinear;
            });
        const auto surface_format =
            aligned_formats.empty() ? formats->front() : aligned_formats.front();

        auto aligned_modes =
            *present_modes |
            std::views::filter([](const hgn::present_mode_khr mode) noexcept -> bool {
                return mode == hgn::present_mode_khr::mailbox;
            });
        const auto present_mode =
            aligned_modes.empty() ? hgn::present_mode_khr::fifo : aligned_modes.front();

        auto extent = capabilities->current_extent();
        if (extent.width == std::numeric_limits<std::uint32_t>::max()) {
            extent = hgn::extent2d{.width = 800, .height = 600};
        }

        auto image_count = capabilities->min_image_count() + 1;
        if (capabilities->min_image_count() > 0 and image_count > capabilities->max_image_count()) {
            image_count = capabilities->max_image_count();
        }

        const auto indices = std::array{
            hard.graphics_queue().queue_family_index(), hard.present_queue().queue_family_index()
        };
        const bool is_duplicate = indices[0] == indices[1];
        auto       builder      = hgn::swapchain_khr::builder{}
                                      .surface(surface)
                                      .min_image_count(image_count)
                                      .image_format(surface_format.format())
                                      .image_color_space(surface_format.color_space())
                                      .image_extent(extent)
                                      .pre_transform(capabilities->current_transform())
                                      .present_mode(present_mode);
        if (is_duplicate) {
            builder.image_sharing_mode(hgn::sharing_mode::exclusive);
        } else {
            builder.image_sharing_mode(hgn::sharing_mode::concurrent).queue_family_indices(indices);
        }
        auto swapchain = builder.build(hard.device());
        if (not swapchain) return make_error(runtime_error, "Failed to make swapchain.");

        auto images = swapchain->get_images_khr(hard.device());
        if (not images) return make_error(runtime_error, "Failed to get images.");

        auto views = std::vector<hgn::image_view>{};
        views.reserve(images->size());

        for (const auto i : std::views::indices(images->size())) {
            auto view = hgn::image_view::builder{}
                            .image((*images)[i])
                            .format(swapchain->image_format())
                            .build(hard.device());
            if (not view) return make_error(runtime_error, "Failed to make image view.");

            views.emplace_back(std::move(*view));
        }

        return image{std::move(*swapchain), std::move(*images), std::move(views)};
    }

  private:
    explicit image(
        hgn::swapchain_khr&&           swapchain,
        std::vector<hgn::image>&&      images,
        std::vector<hgn::image_view>&& views
    ) noexcept
        : swapchain_{std::move(swapchain)}, images_{std::move(images)}, views_{std::move(views)} {}

    hgn::swapchain_khr           swapchain_;
    std::vector<hgn::image>      images_;
    std::vector<hgn::image_view> views_;
};
}  // namespace sui