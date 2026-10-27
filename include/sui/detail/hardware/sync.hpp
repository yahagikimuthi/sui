#pragma once

#include <expected>
#include <utility>

#include <hgn/sync.hpp>
#include <hgn_oop/hardware.hpp>
#include <hgn_oop/sync.hpp>

#include "sui/error.hpp"

namespace sui {
class sync final {
  public:
    [[nodiscard]] static auto try_make(hgn::oop::device& dev) noexcept
        -> std::expected<sync, error> {
        auto sem1  = dev.try_make_semaphore(hgn::semaphore_create_info{});
        auto sem2  = dev.try_make_semaphore(hgn::semaphore_create_info{});
        auto fence = dev.try_make_fence(hgn::fence_create_info{});

        if (not sem1 or not sem2) return make_error(runtime_error, "Failed to make semaphore.");
        if (not fence) return make_error(runtime_error, "Failed to make semaphore");

        return sync{std::move(*sem1), std::move(*sem2), std::move(*fence)};
    }

    [[nodiscard]] auto image_available_semaphore() noexcept -> hgn::oop::semaphore& {
        return image_available_;
    }

    [[nodiscard]] auto render_finished_semaphore() noexcept -> hgn::oop::semaphore& {
        return render_finished_;
    }

    [[nodiscard]] auto in_flight_fence() noexcept -> hgn::oop::fence& { return fence_; }

  private:
    explicit sync(
        hgn::oop::semaphore&& image_available,
        hgn::oop::semaphore&& render_finished,
        hgn::oop::fence&&     fence
    ) noexcept
        : image_available_{std::move(image_available)},
          render_finished_{std::move(render_finished)},
          fence_{std::move(fence)} {}

    hgn::oop::semaphore image_available_;
    hgn::oop::semaphore render_finished_;
    hgn::oop::fence     fence_;
};
}  // namespace sui