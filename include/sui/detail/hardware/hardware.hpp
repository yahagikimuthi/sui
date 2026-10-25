#pragma once

#include <expected>
#include <ranges>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/physical_device.hpp>
#include <hgn/submit.hpp>
#include <hgn/surface.hpp>
#include <hgn/sync.hpp>
#include <kgm/window.hpp>

#include "sui/detail/hardware/device.hpp"
#include "sui/detail/hardware/instance.hpp"
#include "sui/detail/hardware/physical_device.hpp"
#include "sui/detail/hardware/queue.hpp"
#include "sui/detail/hardware/surface.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class hardware final {
    struct queue_family_indices final {
        std::optional<std::uint32_t> graphics_family;
        std::optional<std::uint32_t> present_family;

        [[nodiscard]] auto is_complete() const noexcept -> bool {
            return graphics_family.has_value() and present_family.has_value();
        }
    };

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

    hardware(const hardware&) noexcept                    = delete;
    auto operator=(const hardware&) noexcept -> hardware& = delete;
    hardware(hardware&&) noexcept                         = default;
    auto operator=(hardware&&) noexcept -> hardware&      = default;
    ~hardware() noexcept                                  = default;

    [[nodiscard]] auto try_get_physical_device_surface_capabilities() noexcept
        -> std::optional<hgn::surface_capabilities_khr> {
        const auto res = hgn::try_get_physical_device_surface_capabilities_khr(physical_, surface_);
        if (not res) return std::nullopt;
        return *res;
    }

    [[nodiscard]] auto try_get_physical_device_surface_formats() noexcept
        -> std::optional<std::vector<hgn::surface_format_khr>> {
        const auto res = hgn::try_get_physical_device_surface_formats_khr(physical_, surface_);
        if (not res) return std::nullopt;
        return *res;
    }

    [[nodiscard]] auto try_get_physical_device_surface_present_modes() noexcept
        -> std::optional<std::vector<hgn::present_mode_khr>> {
        const auto res =
            hgn::try_get_physical_device_surface_present_modes_khr(physical_, surface_);
        if (not res) return std::nullopt;
        return *res;
    }

    [[nodiscard]] auto get_surface() noexcept -> surface& { return surface_; }
    [[nodiscard]] auto get_physical() noexcept -> physical_device& { return physical_; }
    [[nodiscard]] auto get_device() noexcept -> device& { return device_; }
    [[nodiscard]] auto get_graphics_queue() noexcept -> queue& { return graphics_; }
    [[nodiscard]] auto get_present_queue() noexcept -> queue& { return present_; }

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
        return find_queue_families(device, surface).is_complete();
    }

    instance        instance_;
    surface         surface_;
    physical_device physical_;
    device          device_;
    queue           graphics_;
    queue           present_;
};
}  // namespace sui::detail