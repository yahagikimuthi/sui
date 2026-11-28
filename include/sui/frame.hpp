#pragma once

#include <cstdint>
#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/command.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/swapchain.hpp>
#include <hgn/sync.hpp>

#include "sui/hardware.hpp"
#include "sui/queue.hpp"

namespace sui {
class frame final {
  public:
    [[nodiscard]] static auto create(hardware& hard) noexcept -> std::expected<frame, akr::error> {
        auto sem1 = hard.device().create_semaphore();
        auto sem2 = hard.device().create_semaphore();
        if (not sem1 or not sem2) return akr::throw_runtime("Failed to create semaphore.");

        auto fence = hard.device().create_fence();
        if (not fence) return akr::throw_runtime("Failed to create fence.");

        auto queue_family = *find_queue_families(hard.physical(), hard.surface());
        auto graphics     = queue_container{hard.device(), queue_family.graphics};
        auto present      = queue_container{hard.device(), queue_family.present};

        return frame{std::move(*sem1), std::move(*sem2), std::move(*fence), graphics, present};
    }

    [[nodiscard]] auto wait() noexcept -> std::expected<void, hgn::result> {
        return in_flight_.wait();
    }

    [[nodiscard]] auto recreate_sync(hgn::device& dev) noexcept -> bool {
        auto sem1 = dev.create_semaphore();
        auto sem2 = dev.create_semaphore();
        if (not sem1 or not sem2) return akr::fail("Failed to crete semaphore.");

        auto fence = dev.create_fence();
        if (not fence) return akr::fail("Failed to create fence.");

        image_available_ = std::move(*sem1);
        render_finished_ = std::move(*sem2);
        in_flight_       = std::move(*fence);
        return true;
    }

    [[nodiscard]] auto reset() noexcept -> std::expected<void, hgn::result> {
        return in_flight_.reset();
    }

    [[nodiscard]] auto submit(hgn::command_buffer& cmd
    ) noexcept -> std::expected<void, hgn::result> {
        return graphics_.queue()
            .get_submitter()
            .command_buffer(cmd)
            .wait_semaphore(image_available_)
            .signal_semaphore(render_finished_)
            .submit(in_flight_);
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

    [[nodiscard]] auto image_available_semaphore() noexcept -> hgn::semaphore& {
        return image_available_;
    }

    [[nodiscard]] auto graphics_queue() noexcept -> queue_container& { return graphics_; }
    [[nodiscard]] auto present_queue() noexcept -> queue_container& { return present_; }

  private:
    frame(
        hgn::semaphore  image_available,
        hgn::semaphore  render_finished,
        hgn::fence      in_flight,
        queue_container graphics,
        queue_container present
    ) noexcept
        : image_available_{std::move(image_available)},
          render_finished_{std::move(render_finished)},
          in_flight_{std::move(in_flight)},
          graphics_{graphics},
          present_{present} {}

    hgn::semaphore  image_available_;
    hgn::semaphore  render_finished_;
    hgn::fence      in_flight_;
    queue_container graphics_;
    queue_container present_;
};
}  // namespace sui