#pragma once

#include <expected>
#include <hgn/device.hpp>
#include <hgn/types.hpp>
#include <span>
#include <utility>

#include "sui/error.hpp"
#include "sui/physical_device.hpp"

namespace sui {
class device final {
  public:
    [[nodiscard]] static auto try_make(physical_device& physical) noexcept
        -> std::expected<device, error> {
        auto queue_priority = 1.f;
        auto queue_info     = hgn::device_queue_create_info{};
        auto indices        = physical.find_queue_families();
        hgn::device_queue_create_info_setter{queue_info}
            .queue_family_index(indices.value())
            .queue_count(1)
            .queue_priorities(queue_priority);

        auto features = hgn::physical_device_features{};
        auto info     = hgn::device_create_info{};
        hgn::device_create_info_setter{info}
            .queue_create_infos(std::span<const hgn::device_queue_create_info>{queue_info})
            .features(features);

        auto native = physical.try_make_device(info);
        if (not native) return make_error(runtime_error, "Failed to create device.");
        return device{*native};
    }

    device(const device&)                             = delete;
    auto operator=(const device&) noexcept -> device& = delete;

    device(device&& other) noexcept : native_{std::exchange(other.native_, nullptr)} {}
    auto operator=(device&& other) noexcept -> device& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, hgn::null_handle);
        return *this;
    }

    ~device() noexcept { destroy(); }

  private:
    explicit device(hgn::device native) noexcept : native_{native} {}

    void destroy() noexcept {
        if (native_ != nullptr) hgn::destroy_device(native_);
    }

    hgn::device native_{hgn::null_handle};
};
}  // namespace sui