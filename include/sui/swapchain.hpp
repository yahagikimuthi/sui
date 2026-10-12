#pragma once

#include <cassert>
#include <optional>
#include <utility>

#include <hgn/swapchain.hpp>

namespace sui {
class swapchain final {
  public:
    explicit swapchain(hgn::swapchain& native, hgn::device& device)
        : native_{native}, device_{device} {}

    swapchain(const swapchain&) noexcept = delete;

    auto operator=(const swapchain&) noexcept -> swapchain& = delete;

    swapchain(swapchain&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)},
          device_{std::exchange(other.device_, std::nullopt)} {}

    auto operator=(swapchain&& other) noexcept -> swapchain& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        device_ = std::exchange(other.device_, std::nullopt);

        return *this;
    }

    ~swapchain() noexcept { destroy(); }

    [[nodiscard]] auto native() noexcept -> hgn::swapchain& { return native_.value(); }

  private:
    void destroy() noexcept {
        if (not native_) return;
        assert(device_);
        hgn::destroy_swapchain_khr(*device_, *native_);
    }

    std::optional<hgn::swapchain&> native_;
    std::optional<hgn::device&>    device_;
};
}  // namespace sui