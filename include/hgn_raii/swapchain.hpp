#pragma once

#include <functional>
#include <optional>

#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/swapchain.hpp>

namespace hgn::raii {
class swapchain_khr final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::swapchain_create_info_khr& info, hgn::device& dev
    ) noexcept -> std::expected<swapchain_khr, hgn::result> {
        auto res = hgn::try_make_swapchain(dev, info);
        if (not res) return std::unexpected{res.error()};

        return swapchain_khr{*res, dev};
    }

    swapchain_khr(const swapchain_khr&) noexcept                    = delete;
    auto operator=(const swapchain_khr&) noexcept -> swapchain_khr& = delete;

    swapchain_khr(swapchain_khr&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(swapchain_khr&& other) noexcept -> swapchain_khr& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;
        return *this;
    }

    ~swapchain_khr() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::swapchain_khr& { return *native_; }

  private:
    explicit swapchain_khr(hgn::swapchain_khr& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_swapchain_khr(dev_, get());
    }

    std::optional<hgn::swapchain_khr&>  native_;
    std::reference_wrapper<hgn::device> dev_;
};

[[nodiscard]] inline auto try_make_swapchain(
    const hgn::swapchain_create_info_khr& info, hgn::device& dev
) noexcept -> std::expected<swapchain_khr, hgn::result> {
    return swapchain_khr::try_make(info, dev);
}
}  // namespace hgn::raii