#pragma once

#include <cstdint>
#include <exception>
#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/allocator.hpp>
#include <hgn/command.hpp>
#include <hgn/device.hpp>
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
        if (not hard) return hard.error().rethrow_runtime("Failed to create hardware object.");

        auto display = display::create(*hard);
        if (not display) return display.error().rethrow_runtime("Failed to create display object.");

        auto sync = sync::create(hard->device());
        if (not sync) return sync.error().rethrow_runtime("Failed to create sync object.");

        auto cmd = command::create(*hard);

        if (not cmd) return cmd.error().rethrow_runtime("Failed to create command object.");

        auto allocator =
            hgn::allocator_creator{}.instance(hard->instance()).device(hard->device()).create();
        if (not allocator) return akr::throw_runtime("Failed to create allocator.");

        return rendering_system{
            std::move(*hard), std::move(*display), std::move(*sync), std::move(*cmd)
        };
    }

    rendering_system(const rendering_system&) noexcept                    = delete;
    auto operator=(const rendering_system&) noexcept -> rendering_system& = delete;

    rendering_system(rendering_system&& other) noexcept
        : hard_{std::move(other.hard_)},
          display_{std::move(other.display_)},
          sync_{std::move(other.sync_)},
          cmd_{std::move(other.cmd_)},
          image_index_{other.image_index_},
          is_valid_{std::exchange(other.is_valid_, false)} {}

    auto operator=(rendering_system&& other) noexcept -> rendering_system& {
        if (this == &other) return *this;

        destroy();

        hard_        = std::move(other.hard_);
        display_     = std::move(other.display_);
        sync_        = std::move(other.sync_);
        cmd_         = std::move(other.cmd_);
        image_index_ = other.image_index_;
        is_valid_    = std::exchange(other.is_valid_, false);
        return *this;
    }

    ~rendering_system() noexcept { destroy(); }

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

        display_.begin_barrier(cmd_.buffer(), image_index_);

        cmd_.buffer()
            .get_begin_rendering_executor()
            .color_image_view(display_.get_view(image_index_))
            .depth_image_view(display_.depth_img_view())
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

        display_.end_barrier(cmd_.buffer(), image_index_);

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

    [[nodiscard]] auto device() noexcept -> hgn::device& { return hard_.device(); }

    [[nodiscard]] auto allocator() noexcept -> hgn::allocator& { return hard_.allocator(); }

    [[nodiscard]] auto swapchain_format() noexcept -> hgn::format {
        return display_.swapchain().image_format();
    }

    [[nodiscard]] auto depth_fmt() const noexcept -> hgn::format { return display_.depth_fmt(); }

    [[nodiscard]] auto command_buffer() noexcept -> hgn::command_buffer& { return cmd_.buffer(); }

  private:
    rendering_system(hardware hard, display display, sync sync, command cmd) noexcept
        : hard_{std::move(hard)},
          display_{std::move(display)},
          sync_{std::move(sync)},
          cmd_{std::move(cmd)} {}

    [[nodiscard]] static auto norm(const std::uint8_t color) noexcept -> float {
        return static_cast<float>(color) / 255.f;
    }

    void destroy() noexcept {
        if (not is_valid_) return;
        if (not hard_.device().wait_idle()) std::terminate();
    }

    hardware      hard_;
    display       display_;
    sync          sync_;
    command       cmd_;
    std::uint32_t image_index_{};
    bool          is_valid_{true};
};
}  // namespace sui