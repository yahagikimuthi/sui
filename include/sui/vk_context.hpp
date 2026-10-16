#pragma once

#include <expected>
#include <iostream>
#include <utility>

#include <hgn/image.hpp>
#include <hgn/submit.hpp>
#include <kgm/window.hpp>

#include "sui/error.hpp"
#include "sui/hardware.hpp"
#include "sui/protocol.hpp"
#include "sui/render_ctx.hpp"

namespace sui {
class vk_context final {
  public:
    [[nodiscard]] static auto try_make(kgm::window& win) noexcept
        -> std::expected<vk_context, error> {
        auto hard_res = hardware::try_make(win);
        if (not hard_res)
            return make_error(runtime_error, "Failed to make hardware object.", hard_res.error());

        auto render_res = render_context::try_make(*hard_res);
        if (not render_res)
            return make_error(
                runtime_error, "Failed to make render context class.", render_res.error()
            );

        auto protocol_res = protocol::try_make(
            hard_res->native_device(), hard_res->get_physical_device().graphics_family()
        );
        if (not protocol_res)
            return make_error(
                runtime_error, "Failed to make protocol object.", protocol_res.error()
            );

        return vk_context{std::move(*hard_res), std::move(*render_res), std::move(*protocol_res)};
    }

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
    explicit vk_context(hardware&& hard, render_context&& render, protocol&& prot) noexcept
        : hard_{std::move(hard)}, render_{std::move(render)}, protocol_{std::move(prot)} {}

    hardware       hard_;
    render_context render_;
    protocol       protocol_;
    std::uint32_t  image_index_{};
};
}  // namespace sui