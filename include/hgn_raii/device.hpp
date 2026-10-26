#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/device.hpp>
#include <hgn/physical_device.hpp>

namespace hgn::raii {
class physical_device final {
  public:
    [[nodiscard]] static auto make(hgn::physical_device&& native) noexcept -> physical_device {
        return physical_device{std::move(native)};
    }

    physical_device(hgn::physical_device&& native) noexcept : native_{native} {}

    [[nodiscard]] auto get() noexcept -> hgn::physical_device& { return native_; }

    [[nodiscard]] operator hgn::physical_device&() noexcept { return get(); }

  private:
    std::reference_wrapper<hgn::physical_device> native_;
};

[[nodiscard]] inline auto make_physical(hgn::physical_device&& native) noexcept -> physical_device {
    return physical_device::make(std::move(native));
}

class device final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::device_create_info& info, hgn::physical_device& physical
    ) noexcept -> std::expected<device, hgn::result> {
        auto res = hgn::try_make_device(physical, info);
        if (not res) return std::unexpected{res.error()};
        return device{*res};
    }

    device(const device&) noexcept                    = delete;
    auto operator=(const device&) noexcept -> device& = delete;

    device(device&& other) noexcept : native_{std::exchange(other.native_, std::nullopt)} {}

    auto operator=(device&& other) noexcept -> device& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        return *this;
    }

    ~device() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::device& { return *native_; }

    [[nodiscard]] operator hgn::device&() noexcept { return get(); }

  private:
    explicit device(hgn::device& native) noexcept : native_{native} {}

    void destroy() noexcept { hgn::destroy_device(get()); }

    std::optional<hgn::device&> native_;
};

[[nodiscard]] inline auto try_make_device(
    const hgn::device_create_info& info, hgn::physical_device& physical
) noexcept -> std::expected<device, hgn::result> {
    return device::try_make(info, physical);
}
}  // namespace hgn::raii