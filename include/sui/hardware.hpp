#pragma once

#include <expected>
#include <optional>
#include <ranges>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/hgn.hpp>
#include <hgn/instance.hpp>
#include <hgn/physical.hpp>
#include <hgn/queue.hpp>
#include <hgn/surface.hpp>
#include <kgm/kgm.hpp>
#include <kgm/window.hpp>

#include "sui/queue.hpp"

namespace sui {
struct queue_families final {
    hgn::queue_family graphics;
    hgn::queue_family present;
};

[[nodiscard]] inline auto find_queue_families(
    hgn::physical_device& physical, hgn::surface_khr& surface
) noexcept -> std::optional<queue_families> {
    auto graphics = std::optional<hgn::queue_family>{};
    auto present  = std::optional<hgn::queue_family>{};

    for (auto& family : physical.get_queue_families()) {
        if ((family.flags() & hgn::queue_flag_bits::graphics) != hgn::queue_flag_bits::none)
            graphics = family;

        const auto supporting = family.is_supporting_surface(surface);
        if (supporting and *supporting) present = family;
    }

    if (graphics and present) return queue_families{.graphics = *graphics, .present = *present};
    return std::nullopt;
}

class hardware final {
  public:
    [[nodiscard]] static auto create(kgm::window& win
    ) noexcept -> std::expected<hardware, akr::error> {
        auto instance = hgn::instance_creator{}
                            .app_name("sui")
                            .app_version(0, 0, 1)
                            .engine_name("no engine")
                            .engine_version(0, 0, 1)
                            .extensions(kgm::get_required_instance_extensions())
#ifndef NDEBUG
                            .debug(true)
#endif
                            .create();
        if (not instance) return akr::throw_runtime("Failed to create instance.");

        auto surface = win.create_surface(*instance);
        if (not surface) return akr::throw_runtime("Failed to create surface.");

        auto physicals = instance->get_physicals();
        if (not physicals) return akr::throw_runtime("Failed to get physical devices.");

        auto suitable_physicals =
            *physicals | std::views::filter([&](hgn::physical_device& physical) noexcept -> bool {
                return find_queue_families(physical, *surface).has_value();
            });

        if (suitable_physicals.empty())
            return akr::throw_runtime("Failed to find suitable physical device.");

        auto physical = suitable_physicals.front();
        auto families = *find_queue_families(physical, *surface);

        auto device = physical.create_device(families.graphics);
        if (not device) return akr::throw_runtime("Failed to create device.");

        auto graphics = queue_container{*device, families.graphics};
        auto present  = queue_container{*device, families.present};

        return hardware{
            std::move(*instance),
            std::move(*surface),
            physical,
            std::move(*device),
            graphics,
            present
        };
    }

    [[nodiscard]] auto instance() noexcept -> hgn::instance& { return instance_; }
    [[nodiscard]] auto surface() noexcept -> hgn::surface_khr& { return surface_; }
    [[nodiscard]] auto physical() noexcept -> hgn::physical_device& { return physical_; }
    [[nodiscard]] auto device() noexcept -> hgn::device& { return dev_; }
    [[nodiscard]] auto graphics_queue() noexcept -> queue_container& { return graphics_; }
    [[nodiscard]] auto present_queue() noexcept -> queue_container& { return present_; }

  private:
    hardware(
        hgn::instance        instance,
        hgn::surface_khr     surface,
        hgn::physical_device physical,
        hgn::device          dev,
        queue_container      graphics,
        queue_container      present
    ) noexcept
        : instance_{std::move(instance)},
          surface_{std::move(surface)},
          physical_{physical},
          dev_{std::move(dev)},
          graphics_{graphics},
          present_{present} {}

    hgn::instance        instance_;
    hgn::surface_khr     surface_;
    hgn::physical_device physical_;
    hgn::device          dev_;
    queue_container      graphics_;
    queue_container      present_;
};
}  // namespace sui