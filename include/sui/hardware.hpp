#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/device.hpp>
#include <hgn/surface.hpp>
#include <kgm/window.hpp>

#include "sui/device.hpp"
#include "sui/queue.hpp"

namespace sui {
class hardware final {
  public:
    [[nodiscard]] static auto create(kgm::window& win
    ) noexcept -> std::expected<hardware, akr::error> {
        auto device = device_container::create(win);
        if (not device)
            return akr::make_error(
                akr::runtime_error, "Failed to create device container.", device.error()
            );

        auto families = *find_queue_families(device->physical(), device->surface());

        auto graphics = queue_container{device->device(), families.graphics};
        auto present  = queue_container{device->device(), families.present};

        return hardware{std::move(*device), graphics, present};
    }

    [[nodiscard]] auto surface() noexcept -> hgn::surface_khr& { return device_.surface(); }

    [[nodiscard]] auto device() noexcept -> hgn::device& { return device_.device(); }

    [[nodiscard]] auto graphics_queue() noexcept -> queue_container& { return graphics_; }

    [[nodiscard]] auto present_queue() noexcept -> queue_container& { return present_; }

  private:
    hardware(
        device_container device, const queue_container& graphics, const queue_container& present
    ) noexcept
        : device_{std::move(device)}, graphics_{graphics}, present_{present} {}

    device_container device_;
    queue_container  graphics_;
    queue_container  present_;
};
}  // namespace sui