#pragma once

#include <cstdint>
#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/allocator.hpp>
#include <hgn/enums.hpp>
#include <hgn/hgn.hpp>
#include <kgm/window.hpp>

#include "sui/command.hpp"
#include "sui/display.hpp"
#include "sui/hardware.hpp"
#include "sui/sync.hpp"

namespace sui {
class rendering_system final {
  public:
    [[nodiscard]] static auto create(kgm::window& win
    ) noexcept -> std::expected<rendering_system, akr::error> {
        auto hard = hardware::create(win);
        if (not hard)
            return akr::make_error(
                akr::runtime_error, "Failed to create hardware object.", hard.error()
            );

        auto display = display::create(*hard);
        if (not display)
            return akr::make_error(
                akr::runtime_error, "Failed to create display object.", display.error()
            );

        auto sync = sync::create(hard->device());
        if (not sync) return akr::make_error(akr::runtime_error, "Failed to create sync objects.");

        auto cmd = command::create(*hard);

        if (not cmd)
            return akr::make_error(akr::runtime_error, "Failed to create command objects.");

        auto allocator =
            hgn::allocator_creator{}.instance(hard->instance()).device(hard->device()).create();
        if (not allocator)
            return akr::make_error(akr::runtime_error, "Failed to create allocator.");

        return rendering_system{
            std::move(*hard),
            std::move(*display),
            std::move(*sync),
            std::move(*cmd),
            std::move(*allocator)
        };
    }

    [[nodiscard]] auto begin(
        const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a
    ) noexcept -> bool {
        if (not sync_.in_flight_fence().wait()) return akr::fail("Failed to wait fence.");
        if (not sync_.in_flight_fence().reset()) return akr::fail("Failed to reset fence.");

        const auto index =
            display_.swapchain().acquire_next_image(sync_.image_available_semaphore());
        if (not index) return akr::fail("Failed to acquire next image index.");
        image_index_ = *index;

        if (not cmd_.buffer().begin()) return akr::fail("Failed to begin command buffer.");

        cmd_.buffer()
            .get_image_memory_barrier_executor()
            .layout(hgn::image_layout::undefined, hgn::image_layout::color_attachment_optimal)
            .image(display_.get_image(image_index_))
            .barrier();

        cmd_.buffer()
            .get_begin_rendering_executor()
            .image_view(display_.get_view(image_index_))
            .clear_color(norm(r), norm(g), norm(b), norm(a))
            .extent(800, 600)
            .begin();

        cmd_.buffer().set_viewport(800, 600);
        cmd_.buffer().set_scissor(800, 600);
        return true;
    }

    [[nodiscard]] auto begin() noexcept -> bool { return begin(0, 0, 0, 255); }

    [[nodiscard]] auto begin(
        const std::uint8_t r, const std::uint8_t g, const std::uint8_t b
    ) noexcept -> bool {
        return begin(r, g, b, 255);
    }

    void render(const std::uint32_t index_count, const std::uint32_t instance_count) noexcept {
        cmd_.buffer().draw_indexed(index_count, instance_count);
    }

    [[nodiscard]] auto end() noexcept -> bool {
        cmd_.buffer().end_rendering();

        cmd_.buffer()
            .get_image_memory_barrier_executor()
            .layout(hgn::image_layout::color_attachment_optimal, hgn::image_layout::present_src_khr)
            .image(display_.get_image(image_index_))
            .barrier();

        if (not cmd_.buffer().end()) return akr::fail("Failed to end command buffer.");

        if (not hard_.graphics_queue()
                    .queue()
                    .get_submitter()
                    .command_buffer(cmd_.buffer())
                    .wait_semaphore(sync_.image_available_semaphore())
                    .signal_semaphore(sync_.render_finished_semaphore())
                    .submit(sync_.in_flight_fence()))
            return akr::fail("Failed to submit.");

        const auto res = hard_.present_queue()
                             .queue()
                             .get_present_executor()
                             .image_index(image_index_)
                             .semaphore(sync_.render_finished_semaphore())
                             .swapchain(display_.swapchain())
                             .present();
        if (res) return true;

        const auto code = res.error();
        if (code == hgn::result::error_out_of_date_khr or code == hgn::result::suboptimal_khr) {
            return akr::fail("Resize is occurring");
        }

        return false;
    }

  private:
    rendering_system(
        hardware hard, display display, sync sync, command cmd, hgn::allocator allocator
    ) noexcept
        : hard_{std::move(hard)},
          display_{std::move(display)},
          sync_{std::move(sync)},
          cmd_{std::move(cmd)},
          allocator_{std::move(allocator)} {}

    [[nodiscard]] static auto norm(const std::uint8_t color) noexcept -> float {
        return static_cast<float>(color) / 255.f;
    }

    hardware       hard_;
    display        display_;
    sync           sync_;
    command        cmd_;
    hgn::allocator allocator_;
    std::uint32_t  image_index_{};
};
}  // namespace sui