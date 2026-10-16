#pragma once

#include <expected>
#include <optional>
#include <utility>

#include <hgn/device.hpp>

#include "hgn/enums.hpp"
#include "sui/error.hpp"
#include "sui/physical_device.hpp"

namespace sui {
class device final {
  public:
    [[nodiscard]] static auto try_make(sui::physical_device& physical) noexcept
        -> std::expected<device, error> {
        auto queue_info = hgn::device_queue_create_info{};
        queue_info.queue_family_index(physical.graphics_family())
            .queue_count(1)
            .queue_priorities(1);

        auto dynamic_feature = hgn::physical_device_dynamic_rendering_features{};

        auto info = hgn::device_create_info{};
        info.queue_create_infos({queue_info})
            .features(hgn::physical_device_features{})
            .extensions({hgn::khr_swapchain_extention_name})
            .dynamic_rendering_features(dynamic_feature);

        auto res = hgn::try_make_device(physical.native(), info);
        if (not res) return make_error(runtime_error, "Failed to make logical device\n");
        return device{res->get()};
    }

    explicit device(hgn::device& native) noexcept : native_{native} {}

    device(const device&) noexcept = delete;

    auto operator=(const device&) noexcept -> device& = delete;

    device(device&& other) noexcept : native_{std::exchange(other.native_, std::nullopt)} {}

    auto operator=(device&& other) noexcept -> device& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);

        return *this;
    }

    ~device() noexcept { destroy(); }

    [[nodiscard]] auto try_wait_idle() noexcept -> bool {
        const auto res = hgn::try_device_wait_idle(*native_);
        return res.has_value();
    }

    [[nodiscard]] auto native() noexcept -> hgn::device& { return *native_; }

  private:
    void destroy() noexcept {
        if (native_) hgn::destroy_device(*native_);
    }

    std::optional<hgn::device&> native_;
};
}  // namespace sui