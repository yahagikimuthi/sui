#pragma once

#include <expected>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <hgn/swapchain.hpp>
#include <hgn_raii/swapchain.hpp>

#include "hgn_oop/hardware.hpp"
#include "hgn_oop/sync.hpp"

namespace hgn::oop {
class swapchain_khr final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::swapchain_create_info_khr& info, oop::device& dev
    ) noexcept -> std::expected<oop::swapchain_khr, hgn::result> {
        auto native = raii::try_make_swapchain(info, dev.get());
        if (not native) return std::unexpected{native.error()};

        return swapchain_khr{std::move(*native)};
    }

    [[nodiscard]] auto try_acquire_next_image(oop::device& dev, oop::semaphore& semapho) noexcept
        -> std::expected<std::uint32_t, hgn::result> {
        return hgn::try_acquire_next_image_khr(dev.get(), native_, semapho.get());
    }

  private:
    explicit swapchain_khr(raii::swapchain_khr&& native) noexcept : native_{std::move(native)} {}

    raii::swapchain_khr native_;
};
}  // namespace hgn::oop