#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/device.hpp>
#include <hgn/sync.hpp>

class sync final {
  public:
    [[nodiscard]] static auto create(hgn::device& dev) noexcept -> std::expected<sync, akr::error> {
        auto sem1  = dev.create_semaphore();
        auto sem2  = dev.create_semaphore();
        auto fence = dev.create_fence();

        if (not sem1 or not sem2)
            return akr::make_error(akr::runtime_error, "Failed to create semaphore.");
        if (not fence) return akr::make_error(akr::runtime_error, "Failed to create fence.");

        return sync{std::move(*sem1), std::move(*sem2), std::move(*fence)};
    }

    [[nodiscard]] auto image_available_semaphore() noexcept -> hgn::semaphore& {
        return image_available_;
    }

    [[nodiscard]] auto render_finished_semaphore() noexcept -> hgn::semaphore& {
        return render_finished_;
    }

    [[nodiscard]] auto in_flight_fence() noexcept -> hgn::fence& { return in_flight_fence_; }

  private:
    sync(
        hgn::semaphore image_available, hgn::semaphore render_finished, hgn::fence in_flight
    ) noexcept
        : image_available_{std::move(image_available)},
          render_finished_{std::move(render_finished)},
          in_flight_fence_{std::move(in_flight)} {}

    hgn::semaphore image_available_;
    hgn::semaphore render_finished_;
    hgn::fence     in_flight_fence_;
};