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
using namespace std::string_literals;

class rendering_system final {
  public:
    [[nodiscard]] static auto create(kgm::window& win
    ) noexcept -> std::expected<rendering_system, akr::error> {
        auto hard = hardware::create(win);
        if (not hard) return hard.error().rethrow_runtime("Failed to create hardware object.");

        auto frame = frame::create(*hard);
        if (not frame) return frame.error().rethrow_runtime("Failed to create frame.");

        auto display = display::create(*hard, *frame);
        if (not display) return display.error().rethrow_runtime("Failed to create display object.");

        auto cmd = command::create(*hard, *frame);

        if (not cmd) return cmd.error().rethrow_runtime("Failed to create command object.");

        auto allocator =
            hgn::allocator_creator{}.instance(hard->instance()).device(hard->device()).create();
        if (not allocator) return akr::throw_runtime("Failed to create allocator.");

        auto depth_buffers = depth_buffer_manager{};

        return rendering_system{
            std::move(*hard),
            std::move(*display),
            std::move(*frame),
            std::move(*cmd),
            std::move(depth_buffers)
        };
    }

    rendering_system(const rendering_system&) noexcept                    = delete;
    auto operator=(const rendering_system&) noexcept -> rendering_system& = delete;

    rendering_system(rendering_system&& other) noexcept
        : hard_{std::move(other.hard_)},
          display_{std::move(other.display_)},
          frame_{std::move(other.frame_)},
          cmd_{std::move(other.cmd_)},
          image_index_{other.image_index_},
          is_valid_{std::exchange(other.is_valid_, false)} {}

    auto operator=(rendering_system&& other) noexcept -> rendering_system& {
        if (this == &other) return *this;

        destroy();

        hard_        = std::move(other.hard_);
        display_     = std::move(other.display_);
        frame_       = std::move(other.frame_);
        cmd_         = std::move(other.cmd_);
        image_index_ = other.image_index_;
        is_valid_    = std::exchange(other.is_valid_, false);
        return *this;
    }

    ~rendering_system() noexcept { destroy(); }

    [[nodiscard]] auto begin(
        const std::uint32_t width,
        const std::uint32_t height,
        const std::uint8_t  r = 0,
        const std::uint8_t  g = 0,
        const std::uint8_t  b = 0,
        const std::uint8_t  a = 255
    ) noexcept -> bool {
        if (width == 0 or height == 0) return true;

        if (not frame_.wait()) return akr::fail("Failed to wait fence.");

        const auto index =
            display_.swapchain().acquire_next_image(frame_.image_available_semaphore());
        if (not index) {
            if (index.error() != hgn::result::error_out_of_date_khr and
                index.error() != hgn::result::suboptimal_khr)
                return akr::fail("Failed to acquire next image.");

            if (not hard_.device().wait_idle()) return akr::fail("Failed to wait device.");

            auto display = display_.recreate(hard_, frame_, width, height);
            if (not display)
                return akr::fail(
                    "Failed to recreate display class because"s + display.error().what()
                );

            display_ = std::move(*display);

            return begin(width, height);
        }

        image_index_ = *index;

        if (not cmd_.buffer().begin()) return akr::fail("Failed to begin command buffer.");

        display_.begin_barrier(cmd_.buffer(), image_index_);

        cmd_.buffer()
            .get_begin_rendering_executor()
            .color_image_view(display_.get_view(image_index_))
            .depth_image_view(display_.depth_img_view())
            .clear_color(
                static_cast<float>(r) / 255.f,
                static_cast<float>(g) / 255.f,
                static_cast<float>(b) / 255.f,
                static_cast<float>(a) / 255.f
            )
            .extent(display_.swapchain().width(), display_.swapchain().height())
            .begin();

        cmd_.buffer().set_viewport(
            static_cast<float>(display_.swapchain().width()),
            static_cast<float>(display_.swapchain().height())
        );
        cmd_.buffer().set_scissor(display_.swapchain().width(), display_.swapchain().height());
        return true;
    }

    void render(const std::uint32_t index_count, const std::uint32_t instance_count) noexcept {
        cmd_.buffer().draw_indexed(index_count, instance_count);
    }

    [[nodiscard]] auto end(const std::uint32_t width, const std::uint32_t height) noexcept -> bool {
        cmd_.buffer().end_rendering();

        display_.end_barrier(cmd_.buffer(), image_index_);

        if (not cmd_.buffer().end()) return akr::fail("Failed to end command buffer.");

        if (not frame_.reset()) return akr::fail("Failed to reset fence.");

        if (not frame_.submit(cmd_.buffer())) return akr::fail("Failed to submit.");

        const auto res = frame_.present(image_index_, display_.swapchain());
        if (res) return true;

        const auto code = res.error();
        if (code != hgn::result::error_out_of_date_khr and code != hgn::result::suboptimal_khr)
            return akr::fail("Failed to present.");

        if (not hard_.device().wait_idle()) return akr::fail("Failed to wait device");

        auto display = display_.recreate(hard_, frame_, width, height);
        if (not display)
            return akr::fail("Failed to recreate display because"s + display.error().what());

        display_ = std::move(*display);
        return true;
    }

    [[nodiscard]] auto device() noexcept -> hgn::device& { return hard_.device(); }

    [[nodiscard]] auto allocator() noexcept -> hgn::allocator& { return hard_.allocator(); }

    [[nodiscard]] auto swapchain_format() noexcept -> hgn::format {
        return display_.swapchain().image_format();
    }

    [[nodiscard]] auto depth_fmt() const noexcept -> hgn::format { return display_.depth_fmt(); }

    [[nodiscard]] auto command_buffer() noexcept -> hgn::command_buffer& { return cmd_.buffer(); }

    [[nodiscard]] auto command_pool() noexcept -> hgn::command_pool& { return cmd_.pool(); }

    [[nodiscard]] auto graphics_queue() noexcept -> hgn::queue& {
        return frame_.graphics_queue().queue();
    }

  private:
    rendering_system(
        hardware hard, display display, frame frame, command cmd, depth_buffer_manager depth_buffers
    ) noexcept
        : hard_{std::move(hard)},
          display_{std::move(display)},
          frame_{std::move(frame)},
          cmd_{std::move(cmd)},
          depth_buffers_{std::move(depth_buffers)} {}

    void destroy() noexcept {
        if (not is_valid_) return;
        if (not hard_.device().wait_idle()) std::terminate();
    }

    hardware             hard_;
    display              display_;
    frame                frame_;
    command              cmd_;
    depth_buffer_manager depth_buffers_;
    std::uint32_t        image_index_{};
    bool                 is_valid_{true};
};
}  // namespace sui
