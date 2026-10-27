#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <optional>
#include <utility>

#include <hgn/device.hpp>
#include <hgn/image.hpp>
#include <hgn/submit.hpp>
#include <hgn/swapchain.hpp>
#include <hgn/sync.hpp>
#include <hgn_raii/device.hpp>
#include <hgn_raii/sync.hpp>

#include "sui/detail/debug_impl.hpp"
#include "sui/detail/others.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class sync final {
  public:
    [[nodiscard]] static auto try_make(hgn::raii::device& dev) noexcept
        -> std::expected<sync, error> {
        auto sem1 = hgn::raii::try_make_semaphore(hgn::semaphore_create_info{}, dev);
        auto sem2 = hgn::raii::try_make_semaphore(hgn::semaphore_create_info{}, dev);
        auto fen  = hgn::raii::try_make_fence(hgn::fence_create_info{}, dev);

        if (not sem1) return make_error(runtime_error, "Failed to make image available semaphore.");
        if (not sem2) return make_error(runtime_error, "Failed to make render finished semaphore.");
        if (not fen) return make_error(runtime_error, "Failed to make fence.");

        return sync{std::move(*sem1), std::move(*sem2), std::move(*fen)};
    }

    sync(const sync&) noexcept                    = delete;
    auto operator=(const sync&) noexcept -> sync& = delete;
    sync(sync&&) noexcept                         = default;
    auto operator=(sync&&) noexcept -> sync&      = default;
    ~sync() noexcept                              = default;

    [[nodiscard]] auto try_wait_fence(hgn::device& dev) noexcept -> bool {
        auto wait_res = hgn::try_wait_for_fence(dev, fence_);
        if (not wait_res) return fail("Failed to wait for fence.");

        auto reset_res = hgn::try_reset_fence(dev, fence_);
        if (not reset_res) return fail("Failed to reset fence.");

        return true;
    }

    [[nodiscard]] auto try_submit(hgn::queue& graphics_queue, const hgn::submit_info& info) noexcept
        -> bool {
        return hgn::try_queue_submit(graphics_queue, info, fence_).has_value();
    }

    [[nodiscard]] auto make_present_info(
        hgn::swapchain_khr& chain, const std::uint32_t image_index
    ) noexcept -> hgn::present_info_khr {
        return hgn::present_info_khr{}
            .setting_swapchain(chain)
            .image_indices(image_index)
            .wait_semaphore(render_finished_);
    }

    [[nodiscard]] auto image_available_semaphore() noexcept -> hgn::semaphore& {
        return image_available_.get();
    }

    [[nodiscard]] auto render_finished_semaphore() noexcept -> hgn::semaphore& {
        return render_finished_.get();
    }

    [[nodiscard]] auto try_acquire_next_image_khr(
        hgn::device& dev, hgn::swapchain_khr& chain
    ) noexcept -> std::optional<std::uint32_t> {
        auto res = hgn::try_acquire_next_image_khr(dev, chain, image_available_);
        return to_optional(res);
    }

  private:
    explicit sync(
        hgn::raii::semaphore&& image, hgn::raii::semaphore&& render, hgn::raii::fence&& fen
    ) noexcept
        : image_available_{std::move(image)},
          render_finished_{std::move(render)},
          fence_{std::move(fen)} {}

    hgn::raii::semaphore image_available_;
    hgn::raii::semaphore render_finished_;
    hgn::raii::fence     fence_;
};
}  // namespace sui::detail