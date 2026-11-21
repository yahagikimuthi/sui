#pragma once

#include <expected>
#include <ranges>
#include <utility>
#include <vector>

#include <akr/akr.hpp>
#include <hgn/command.hpp>
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
        if (not swapchain) return akr::throw_runtime("Failed to create swapchain.");

        auto images = swapchain->get_images();
        if (not images) return akr::throw_runtime("Failed to get images from swapchain.");

        auto views = std::vector<hgn::image_view>{};
        for (auto& image : *images) {
            auto view = dev.get_image_view_creator()
                            .aspect_mask(hgn::image_aspect_flag_bits::color)
                            .image(image)
                            .format(swapchain->image_format())
                            .create();
            if (not view) return akr::throw_runtime("Failed to create image view.");

            views.emplace_back(std::move(*view));
        }

        auto aligned_depth_formats =
            std::array{
                hgn::format::d32_sfloat,
                hgn::format::d24_unorm_s8_uint,
                hgn::format::d32_sfloat_s8_uint
            } |
            std::views::filter([&](hgn::format fmt) noexcept -> bool {
                return hard.physical().is_supporting_depth_format(fmt);
            });
        if (aligned_depth_formats.empty())
            return akr::throw_runtime("Failed to find aligned depth format.");

        auto depth_fmt = aligned_depth_formats.front();

        auto depth_img = hard.allocator()
                             .get_image_creator()
                             .extent(800, 600)
                             .format(depth_fmt)
                             .usage(hgn::image_usage_flag_bits::depth_stencil_attachment)
                             .create();
        if (not depth_img) return akr::throw_runtime("Failed to create depth image.");

        auto depth_view = dev.get_image_view_creator()
                              .image(*depth_img)
                              .aspect_mask(hgn::image_aspect_flag_bits::depth)
                              .format(depth_fmt)
                              .create();
        if (not depth_view) return akr::throw_runtime("Failed to create depth image view.");

        return display{
            std::move(*swapchain),
            std::move(*depth_img),
            std::move(*depth_view),
            depth_fmt,
            *images,
            std::move(views)
        };
    }

    [[nodiscard]] auto recreate(
        hardware& hard, const std::uint32_t width, const std::uint32_t height
    ) noexcept -> std::expected<display, akr::error> {
        auto swapchain =
            hard.device()
                .get_swapchain_creator(hard.surface())
                .extent(width, height)
                .present_mode(hgn::present_mode_khr::mailbox)
                .surface_format(hgn::format::b8g8r8a8_srgb, hgn::color_space_khr::srgb_nonlinear)
                .queue_families(hard.graphics_queue().family(), hard.present_queue().family())
                .old_swapchain(swapchain_)
                .create();
        if (not swapchain) return akr::throw_runtime("Failed to recreate swapchain.");

        auto images = swapchain->get_images();
        if (not images) return akr::throw_runtime("Failed to get images from swapchain.");

        auto views = std::vector<hgn::image_view>{};
        views.reserve(images->size());
        for (auto& image : *images) {
            auto view = hard.device()
                            .get_image_view_creator()
                            .image(image)
                            .aspect_mask(hgn::image_aspect_flag_bits::color)
                            .format(swapchain->image_format())
                            .create();
            if (not view) return akr::throw_runtime("Failed to recreate image view.");
            views.emplace_back(std::move(*view));
        }

        auto aligned_depth_formats =
            std::array{
                hgn::format::d32_sfloat,
                hgn::format::d24_unorm_s8_uint,
                hgn::format::d32_sfloat_s8_uint
            } |
            std::views::filter([&](hgn::format fmt) noexcept -> bool {
                return hard.physical().is_supporting_depth_format(fmt);
            });
        if (aligned_depth_formats.empty())
            return akr::throw_runtime("Failed to find aligned depth format.");

        auto depth_fmt = aligned_depth_formats.front();

        auto depth_img = hard.allocator()
                             .get_image_creator()
                             .extent(swapchain->width(), swapchain->height())
                             .format(depth_fmt)
                             .usage(hgn::image_usage_flag_bits::depth_stencil_attachment)
                             .create();
        if (not depth_img) return akr::throw_runtime("Failed to create depth image.");

        auto depth_view = hard.device()
                              .get_image_view_creator()
                              .image(*depth_img)
                              .aspect_mask(hgn::image_aspect_flag_bits::depth)
                              .format(depth_fmt)
                              .create();
        if (not depth_view) return akr::throw_runtime("Failed to create depth image view.");

        return display{
            std::move(*swapchain),
            std::move(*depth_img),
            std::move(*depth_view),
            depth_fmt,
            std::move(*images),
            std::move(views)
        };
    }

    [[nodiscard]] auto swapchain() noexcept -> hgn::swapchain_khr& { return swapchain_; }

    [[nodiscard]] auto depth_fmt() const noexcept -> hgn::format { return depth_fmt_; }

    [[nodiscard]] auto depth_img() noexcept -> hgn::image& { return depth_img_; }

    [[nodiscard]] auto depth_img_view() noexcept -> hgn::image_view& { return depth_view_; }

    [[nodiscard]] auto get_image(const std::uint32_t i) noexcept -> hgn::borrowed_image& {
        return images_[i];
    }

    [[nodiscard]] auto get_view(const std::uint32_t i) noexcept -> hgn::image_view& {
        return views_[i];
    }

    void begin_barrier(hgn::command_buffer& cmd, const std::uint32_t image_index) noexcept {
        cmd.get_image_memory_barrier_executor()
            .aspect_mask(hgn::image_aspect_flag_bits::color)
            .image(images_[image_index])
            .layout(hgn::image_layout::undefined, hgn::image_layout::color_attachment_optimal)
            .barrier();

        auto aspect = hgn::image_aspect_flag_bits::depth;
        if (depth_fmt_ == hgn::format::d32_sfloat_s8_uint or
            depth_fmt_ == hgn::format::d24_unorm_s8_uint)
            aspect |= hgn::image_aspect_flag_bits::stencil;

        cmd.get_image_memory_barrier_executor()
            .aspect_mask(aspect)
            .image(depth_img_)
            .layout(hgn::image_layout::undefined, hgn::image_layout::depth_attachment_optimal)
            .barrier();
    }

    void end_barrier(hgn::command_buffer& cmd, const std::uint32_t image_index) noexcept {
        cmd.get_image_memory_barrier_executor()
            .aspect_mask(hgn::image_aspect_flag_bits::color)
            .image(images_[image_index])
            .layout(hgn::image_layout::color_attachment_optimal, hgn::image_layout::present_src_khr)
            .barrier();
    }

  private:
    display(
        hgn::swapchain_khr               swapchain,
        hgn::image                       depth_img,
        hgn::image_view                  depth_view,
        const hgn::format                depth_fmt,
        std::vector<hgn::borrowed_image> images,
        std::vector<hgn::image_view>     views
    ) noexcept
        : swapchain_{std::move(swapchain)},
          depth_img_{std::move(depth_img)},
          depth_view_{std::move(depth_view)},
          depth_fmt_{depth_fmt},
          images_{std::move(images)},
          views_{std::move(views)} {}

    hgn::swapchain_khr               swapchain_;
    hgn::image                       depth_img_;
    hgn::image_view                  depth_view_;
    hgn::format                      depth_fmt_;
    std::vector<hgn::borrowed_image> images_;
    std::vector<hgn::image_view>     views_;
};
}  // namespace sui