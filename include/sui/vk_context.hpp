#pragma once

#include <cstdint>
#include <exception>
#include <expected>
#include <iostream>
#include <span>
#include <utility>

#include <hgn/command.hpp>
#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <hgn/submit.hpp>
#include <kgm/window.hpp>

#include "sui/buffer.hpp"
#include "sui/error.hpp"
#include "sui/hardware.hpp"
#include "sui/pipeline.hpp"
#include "sui/protocol.hpp"
#include "sui/swapping.hpp"

namespace sui {
class vk_context final {
  public:
    [[nodiscard]] static auto try_make(kgm::window& win) noexcept
        -> std::expected<vk_context, error> {
        auto hard_res = hardware::try_make(win);
        if (not hard_res)
            return make_error(runtime_error, "Failed to make hardware object.", hard_res.error());

        auto swapping_res = swapping::try_make(*hard_res);
        if (not swapping_res)
            return make_error(
                runtime_error, "Failed to make render context class.", swapping_res.error()
            );

        auto protocol_res = protocol::try_make(
            hard_res->native_device(), hard_res->get_physical_device().graphics_family()
        );
        if (not protocol_res)
            return make_error(
                runtime_error, "Failed to make protocol object.", protocol_res.error()
            );

        auto buffer_res =
            buffer::try_make(hard_res->native_device(), hard_res->get_physical_device().get());
        if (not buffer_res)
            return make_error(runtime_error, "Failed to make buffer.", buffer_res.error());

        auto pipe_res =
            pipeline::try_make(hard_res->native_device(), swapping_res->setting_format());
        if (not pipe_res)
            return make_error(runtime_error, "Failed to make pipeline.", pipe_res.error());

        return vk_context{
            std::move(*hard_res),
            std::move(*swapping_res),
            std::move(*protocol_res),
            std::move(*buffer_res),
            std::move(*pipe_res)
        };
    }

    vk_context(const vk_context&) noexcept = delete;

    auto operator=(const vk_context&) noexcept -> vk_context& = delete;

    vk_context(vk_context&& other) noexcept
        : hard_{std::move(other.hard_)},
          render_{std::move(other.render_)},
          protocol_{std::move(other.protocol_)},
          buffer_{std::move(other.buffer_)},
          pipe_{std::move(other.pipe_)},
          image_index_{other.image_index_},
          is_valid_{std::exchange(other.is_valid_, false)} {}

    auto operator=(vk_context&& other) noexcept -> vk_context& {
        if (this == &other) return *this;

        destroy();

        hard_        = std::move(other.hard_);
        render_      = std::move(other.render_);
        protocol_    = std::move(other.protocol_);
        buffer_      = std::move(other.buffer_);
        pipe_        = std::move(other.pipe_);
        image_index_ = other.image_index_;

        is_valid_ = std::exchange(other.is_valid_, false);

        return *this;
    }

    ~vk_context() noexcept { destroy(); }

    [[nodiscard]] auto try_begin_rendering() noexcept -> bool {
        if (not protocol_.try_wait(hard_.native_device())) return false;

        const auto image_index_res = hgn::try_acquire_next_image_khr(
            hard_.native_device(), render_.native_swapchain(), protocol_.image_available_semaphore()
        );

        if (not image_index_res) return false;
        image_index_ = *image_index_res;

        return protocol_.try_begin_rendering(
            render_.native_image(image_index_), render_.native_image_view(image_index_)
        );
    }

    void try_rendering(const std::span<const float> vertices) noexcept {
        buffer_.update(vertices);
        hgn::command_bind_vertex_buffers(
            protocol_.get_command_buffer(), buffer_.get_native_buffer()
        );
        const auto vertex_count = static_cast<std::uint32_t>(vertices.size());
        hgn::command_draw(protocol_.get_command_buffer(), vertex_count, 1);
    }

    void render() noexcept {}

    [[nodiscard]] auto try_end_rendering() noexcept -> bool {
        if (not protocol_.try_end_rendering(render_.native_image(image_index_))) return false;

        const auto submit_info = protocol_.make_submit_info();
        if (not hard_.try_submit(submit_info, protocol_.in_flight_fence())) return false;

        auto present_info = hgn::present_info_khr{};
        present_info.wait_semaphore(protocol_.render_finished_semaphore())
            .setting_swapchain(render_.native_swapchain())
            .image_indices(image_index_);

        const auto res = hard_.try_present(present_info);
        if (res) return true;

        const auto error_code = res.error();
        if (error_code != hgn::result::error_out_of_date_khr and
            error_code != hgn::result::suboptimal_khr) {
            return false;
        }

        std::cout << "Resize\n";
        return true;
    }

  private:
    explicit vk_context(
        hardware&& hard, swapping&& render, protocol&& prot, buffer&& buf, pipeline&& pipe
    ) noexcept
        : hard_{std::move(hard)},
          render_{std::move(render)},
          protocol_{std::move(prot)},
          buffer_{std::move(buf)},
          pipe_{std::move(pipe)} {}

    void destroy() noexcept {
        if (not is_valid_) return;

        if (not hard_.try_device_wait_idle()) {
            std::cerr << "Failed to wait device\n";
            std::terminate();
        }
    }

    hardware      hard_;
    swapping      render_;
    protocol      protocol_;
    buffer        buffer_;
    pipeline      pipe_;
    std::uint32_t image_index_{};
    bool          is_valid_{true};
};
}  // namespace sui