#pragma once

#include <cstdint>
#include <exception>
#include <expected>
#include <span>
#include <utility>

#include <hgn/enums.hpp>

#include "sui/command.hpp"
#include "sui/error.hpp"
#include "sui/hardware.hpp"
#include "sui/image.hpp"
#include "sui/renderer.hpp"
#include "sui/vertex.hpp"

namespace sui {
class context final {
  public:
    [[nodiscard]] static auto make(hgn::window& win) noexcept -> std::expected<context, error> {
        auto hard = hardware::make(win);
        if (not hard) return make_error(runtime_error, "Failed to make hardware.", hard.error());

        auto image = image::make(*hard);
        if (not image) return make_error(runtime_error, "Failed to make image.", image.error());

        auto cmd = command::make(*hard);
        if (not cmd) return make_error(runtime_error, "Failed to make command.", cmd.error());

        auto render = renderer::make(*hard, image->swapchain().image_format());
        if (not render)
            return make_error(runtime_error, "Failed to make renderer.", render.error());

        return context{std::move(*hard), std::move(*image), std::move(*cmd), std::move(*render)};
    }

    context(const context&) noexcept                    = delete;
    auto operator=(const context&) noexcept -> context& = delete;

    context(context&& other) noexcept
        : hard_{std::move(other.hard_)},
          image_{std::move(other.image_)},
          cmd_{std::move(other.cmd_)},
          render_{std::move(other.render_)},
          is_valid_{std::exchange(other.is_valid_, false)} {}

    auto operator=(context&& other) noexcept -> context& {
        if (this == &other) return *this;

        destroy();

        hard_     = std::move(other.hard_);
        image_    = std::move(other.image_);
        cmd_      = std::move(other.cmd_);
        render_   = std::move(other.render_);
        is_valid_ = std::exchange(other.is_valid_, false);
        return *this;
    }

    ~context() noexcept { destroy(); }

    [[nodiscard]] auto begin_rendering() noexcept -> bool {
        if (not hard_.in_flight_fence().wait()) return false;
        if (not hard_.in_flight_fence().reset()) return false;

        const auto image_index =
            image_.swapchain().acquire_next_image(hard_.image_available_semaphore());
        if (not image_index) return false;
        image_index_ = *image_index;

        if (not cmd_.buffer().begin_command()) return false;
        cmd_.buffer()
            .get_image_memory_barrier_executor()
            .layout(hgn::image_layout::undefined, hgn::image_layout::color_attachment_optimal)
            .image(image_.get_image(image_index_))
            .execute();

        cmd_.buffer()
            .get_begin_rendering_executor()
            .image_view(image_.get_view(image_index_))
            .clear_color(0.1f, 0.2f, 0.4f, 1.f)
            .extent(800, 600)
            .execute();

        cmd_.buffer().bind(render_.pipeline());

        cmd_.buffer().set_viewport(800, 600);
        cmd_.buffer().set_scissor(800, 600);
        return true;
    }

    void render(const std::span<const vertex> vertices) noexcept {
        render_.update(vertices);
        cmd_.buffer().bind(render_.buffer());
        const auto vertex_count = static_cast<std::uint32_t>(vertices.size());
        cmd_.buffer().draw(vertex_count, 1);
    }

    [[nodiscard]] auto end_rendering() noexcept -> bool {
        cmd_.buffer().end_rendering();
        cmd_.buffer()
            .get_image_memory_barrier_executor()
            .layout(hgn::image_layout::color_attachment_optimal, hgn::image_layout::present_src_khr)
            .image(image_.get_image(image_index_))
            .execute();
        if (not cmd_.buffer().end()) return false;

        if (not hard_.graphics_queue()
                    .get_submit_executor()
                    .command_buffer(cmd_.buffer())
                    .wait_semaphore(hard_.image_available_semaphore())
                    .signal_semaphore(hard_.render_finished_semaphore())
                    .submit(hard_.in_flight_fence()))
            return false;

        const auto res = hard_.present_queue()
                             .get_present_executor()
                             .image_index(image_index_)
                             .semaphore(hard_.render_finished_semaphore())
                             .swapchain(image_.swapchain())
                             .execute();
        if (res) return true;

        const auto code = res.error();
        if (code == hgn::result::error_out_of_date_khr or code == hgn::result::suboptimal_khr) {
            // Resize
        }
        return false;
    }

  private:
    explicit context(hardware&& hard, image&& image, command&& cmd, renderer&& render) noexcept
        : hard_{std::move(hard)},
          image_{std::move(image)},
          cmd_{std::move(cmd)},
          render_{std::move(render)} {}

    void destroy() noexcept {
        if (not is_valid_) return;
        if (not hard_.device().wait_idle()) std::terminate();
    }

    hardware      hard_;
    image         image_;
    command       cmd_;
    renderer      render_;
    std::uint32_t image_index_{};
    bool          is_valid_{true};
};
}  // namespace sui