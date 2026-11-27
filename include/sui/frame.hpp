#pragma once

#include <cstdint>
#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/command.hpp>
#include <hgn/enums.hpp>
#include <hgn/swapchain.hpp>
#include <hgn/sync.hpp>

#include "wk/detail/hardware.hpp"
#include "wk/detail/queue.hpp"

namespace wk::detail {
class frame final {
  public:
    [[nodiscard]] static auto create(hardware& hard) noexcept -> std::expected<frame, akr::error> {
        auto& dev = hard.device();

        auto sem1 = dev.create_semaphore();
        auto sem2 = dev.create_semaphore();
        if (not sem1 or not sem2) return akr::throw_runtime("Failed to create semaphore.");

        auto fence = dev.create_fence();
        if (not fence) return akr::throw_runtime("Failed to create fence.");

        auto families = *find_queue_families(hard.physical(), hard.surface());

        auto device = hard.physical().create_device(families.graphics);
        if (not device) return akr::throw_runtime("Failed to create device.");

        auto graphics = queue_container{*device, families.graphics};
        auto present  = queue_container{*device, families.present};

        return frame{std::move(*sem1), std::move(*sem2), std::move(*fence), graphics, present};
    }

    [[nodiscard]] auto image_available_semaphore() noexcept -> hgn::semaphore& {
        return image_available_;
    }

    [[nodiscard]] auto graphics_queue() noexcept -> queue_container& { return graphics_; }
    [[nodiscard]] auto present_queue() noexcept -> queue_container& { return present_; }

    [[nodiscard]] auto wait() noexcept -> bool { return in_flight_fence_.wait().has_value(); }
    [[nodiscard]] auto reset() noexcept -> bool { return in_flight_fence_.reset().has_value(); }

    [[nodiscard]] auto submit(hgn::command_buffer& cmd
    ) noexcept -> std::expected<void, hgn::result> {
        return graphics_.queue()
            .get_submitter()
            .command_buffer(cmd)
            .wait_semaphore(image_available_)
            .signal_semaphore(render_finished_)
            .submit(in_flight_fence_);
    }

    [[nodiscard]] auto present(
        const std::uint32_t image_index, hgn::swapchain_khr& swapchain
    ) noexcept -> std::expected<void, hgn::result> {
        return present_.queue()
            .get_present_executor()
            .image_index(image_index)
            .semaphore(render_finished_)
            .swapchain(swapchain)
            .present();
    }

  private:
    frame(
        hgn::semaphore  image_available,
        hgn::semaphore  render_finished,
        hgn::fence      in_flight_fence,
        queue_container graphics,
        queue_container present
    ) noexcept
        : image_available_{std::move(image_available)},
          render_finished_{std::move(render_finished)},
          in_flight_fence_{std::move(in_flight_fence)},
          graphics_{graphics},
          present_{present} {}

    hgn::semaphore  image_available_;
    hgn::semaphore  render_finished_;
    hgn::fence      in_flight_fence_;
    queue_container graphics_;
    queue_container present_;
};
}  // namespace wk::detail