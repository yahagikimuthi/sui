#pragma once

#include <expected>
#include <ranges>
#include <utility>

#include <hgn/physical_device.hpp>
#include <hgn/submit.hpp>
#include <hgn/surface.hpp>
#include <hgn/sync.hpp>
#include <kgm/window.hpp>

#include "sui/device.hpp"
#include "sui/error.hpp"
#include "sui/instance.hpp"
#include "sui/physical_device.hpp"
#include "sui/queue.hpp"
#include "sui/surface.hpp"

namespace sui {
struct queue_family_indices final {
    std::optional<std::uint32_t> graphics_family;
    std::optional<std::uint32_t> present_family;

    [[nodiscard]] auto is_complete() const noexcept -> bool {
        return graphics_family.has_value() and present_family.has_value();
    }
};

class hardware final {
  public:
    [[nodiscard]] static auto try_make(kgm::window& win) noexcept
        -> std::expected<hardware, error> {
        auto inst = instance::try_make();
        if (not inst) return make_error(runtime_error, "Failed to make instance.", inst.error());

        auto sur = surface::try_make(win, inst->get());
        if (not sur) return make_error(runtime_error, "Failed to make surface.", sur.error());

        auto physical_devices_res = inst->try_enumerate_physical_devices();
        if (not physical_devices_res)
            return make_error(runtime_error, "Failed to enumerate physical devices.");
        auto& native_physical_devices = *physical_devices_res;

        auto suitable_physicals =
            native_physical_devices |
            std::views::filter([&](hgn::physical_device& physical) noexcept -> bool {
                return is_device_suitable(physical, sur->get());
            });

        if (suitable_physicals.empty())
            return make_error(runtime_error, "Failed to find a suitable GPU.");

        const auto indices = find_queue_families(suitable_physicals.front(), sur->get());

        auto physical = physical_device{
            suitable_physicals.front(), *indices.graphics_family, *indices.present_family
        };

        auto dev = device::try_make(physical);
        if (not dev) return make_error(runtime_error, "Failed to make device.", dev.error());

        auto graphics_queue = queue::try_make(dev->get(), physical.graphics_family());
        if (not graphics_queue)
            return make_error(
                runtime_error, "Failed to make graphics queue.", graphics_queue.error()
            );

        auto present_queue = queue::try_make(dev->get(), physical.present_family());
        if (not present_queue)
            return make_error(
                runtime_error, "Failed to make present queue.", present_queue.error()
            );

        return hardware{
            std::move(*inst),
            std::move(*sur),
            physical,
            std::move(*dev),
            *graphics_queue,
            *present_queue
        };
    }

    [[nodiscard]] auto try_device_wait_idle() noexcept -> bool { return device_.try_wait_idle(); }

    [[nodiscard]] auto native_surface() noexcept -> hgn::surface_khr& { return surface_.get(); }

    [[nodiscard]] auto get_physical_device() noexcept -> sui::physical_device& { return physical_; }

    [[nodiscard]] auto native_device() noexcept -> hgn::device& { return device_.get(); }

    [[nodiscard]] auto try_submit(const hgn::submit_info& info, hgn::fence& fence) noexcept
        -> bool {
        return graphics_.try_submit(info, fence);
    }

    [[nodiscard]] auto try_present(const hgn::present_info_khr& info) noexcept {
        return present_.try_present(info);
    }

  private:
    explicit hardware(
        instance&&             inst,
        surface&&              sur,
        const physical_device& physical,
        device&&               dev,
        const queue&           graphics,
        const queue&           present
    ) noexcept
        : instance_{std::move(inst)},
          surface_{std::move(sur)},
          physical_{physical},
          device_{std::move(dev)},
          graphics_{graphics},
          present_{present} {}

    [[nodiscard]] static auto find_queue_families(
        hgn::physical_device& device, hgn::surface_khr& surface
    ) noexcept -> queue_family_indices {
        auto indices = queue_family_indices{};

        auto queue_families = hgn::get_physical_device_queue_family_properties(device);

        for (const auto [i, queue_family] : std::views::enumerate(queue_families)) {
            if ((queue_family.flags() & hgn::queue_flag_bits::graphics) !=
                hgn::queue_flag_bits::none) {
                indices.graphics_family = i;
            }

            if (hgn::is_physical_device_surface_support_khr(
                    device, static_cast<std::uint32_t>(i), surface
                )) {
                indices.present_family = i;
            }

            if (indices.is_complete()) break;
        }

        return indices;
    }

    [[nodiscard]] static auto is_device_suitable(
        hgn::physical_device& device, hgn::surface_khr& surface
    ) noexcept -> bool {
        auto indices = find_queue_families(device, surface);
        return indices.is_complete();
    }

    instance        instance_;
    surface         surface_;
    physical_device physical_;
    device          device_;
    queue           graphics_;
    queue           present_;
};
}  // namespace sui