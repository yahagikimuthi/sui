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
#include <hgn/pipeline.hpp>
#include <hgn/submit.hpp>
#include <kgm/window.hpp>

#include "sui/detail/hardware/hardware.hpp"
#include "sui/detail/protocol/protocol.hpp"
#include "sui/detail/rendering/renderer.hpp"
#include "sui/detail/swapping/swapping.hpp"
#include "sui/error.hpp"

namespace sui::detail {
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

        auto protocol_res =
            protocol::try_make(hard_res->get_device(), hard_res->get_physical().graphics_family());
        if (not protocol_res)
            return make_error(
                runtime_error, "Failed to make protocol object.", protocol_res.error()
            );

        auto renderer_res = renderer::try_make(*hard_res, swapping_res->setting_format());
        if (not renderer_res)
            return make_error(runtime_error, "Failed to make renderer.", renderer_res.error());

        return vk_context{
            std::move(*hard_res),
            std::move(*swapping_res),
            std::move(*renderer_res),
            std::move(*protocol_res)
        };
    }

    vk_context(const vk_context&) noexcept                    = delete;
    auto operator=(const vk_context&) noexcept -> vk_context& = delete;

    vk_context(vk_context&& other) noexcept
        : hard_{std::move(other.hard_)},
          swapper_{std::move(other.swapper_)},
          renderer_{std::move(other.renderer_)},
          protocol_{std::move(other.protocol_)},
          image_index_{other.image_index_},
          is_valid_{std::exchange(other.is_valid_, false)} {}

    auto operator=(vk_context&& other) noexcept -> vk_context& {
        if (this == &other) return *this;

        destroy();

        hard_        = std::move(other.hard_);
        swapper_     = std::move(other.swapper_);
        renderer_    = std::move(other.renderer_);
        protocol_    = std::move(other.protocol_);
        image_index_ = other.image_index_;
        is_valid_    = std::exchange(other.is_valid_, false);

        return *this;
    }

    ~vk_context() noexcept { destroy(); }

    [[nodiscard]] auto try_begin_rendering() noexcept -> bool {
        if (not protocol_.try_wait(hard_.get_device())) return false;

        const auto image_index_res =
            protocol_.acquire_next_image(hard_.get_device(), swapper_.native_swapchain());

        if (not image_index_res) return false;
        image_index_ = *image_index_res;

        if (not protocol_.try_begin_rendering(
                swapper_.native_image(image_index_), swapper_.native_image_view(image_index_)
            ))
            return false;

        protocol_.command_bind_pipeline(renderer_.get_pipeline());
        const auto viewport = hgn::viewport{}.width(800).height(600).min_depth(0.f).max_depth(1.f);
        protocol_.command_set_viewport(viewport);

        const auto scissor = hgn::rect2d{}.offset(0, 0).extent(800, 600);
        protocol_.command_set_scissor(scissor);
        return true;
    }

    void try_rendering(const std::span<const vertex> vertices) noexcept {
        renderer_.update(vertices);
        protocol_.command_bind_vertex_buffer(renderer_.get_buffer());
        const auto vertex_count = static_cast<std::uint32_t>(vertices.size());
        protocol_.command_draw(vertex_count, 1);
    }

    [[nodiscard]] auto try_end_rendering() noexcept -> bool {
        if (not protocol_.try_end_rendering(swapper_.native_image(image_index_))) return false;

        const auto submit_info = protocol_.make_submit_info();
        if (not hard_.get_graphics_queue().try_submit(submit_info, protocol_.in_flight_fence()))
            return false;

        const auto present_info = hgn::present_info_khr{}
                                      .wait_semaphore(protocol_.render_finished_semaphore())
                                      .setting_swapchain(swapper_.native_swapchain())
                                      .image_indices(image_index_);

        const auto res = hard_.get_present_queue().try_present(present_info);
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
        hardware&& hard, swapping&& swapper, renderer&& render, protocol&& prot
    ) noexcept
        : hard_{std::move(hard)},
          swapper_{std::move(swapper)},
          renderer_{std::move(render)},
          protocol_{std::move(prot)} {}

    void destroy() noexcept {
        if (not is_valid_) return;

        if (not hard_.get_device().try_wait_idle()) {
            std::cerr << "Failed to wait device\n";
            std::terminate();
        }
    }

    hardware      hard_;
    swapping      swapper_;
    renderer      renderer_;
    protocol      protocol_;
    std::uint32_t image_index_{};
    bool          is_valid_{true};
};
}  // namespace sui::detail