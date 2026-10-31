#pragma once

#include <expected>
#include <optional>
#include <ranges>
#include <utility>

#include <hgn/application.hpp>
#include <hgn/device.hpp>
#include <hgn/device_queue.hpp>
#include <hgn/instance.hpp>
#include <hgn/physical.hpp>
#include <hgn/queue.hpp>
#include <hgn/surface.hpp>
#include <hgn/sync.hpp>
#include <hgn/window.hpp>

#include "sui/error.hpp"

namespace sui {
class hardware final {
    struct queue_family_indices final {
        std::optional<std::uint32_t> graphics_family;
        std::optional<std::uint32_t> present_family;

        [[nodiscard]] auto is_complete() const noexcept -> bool {
            return graphics_family.has_value() and present_family.has_value();
        }
    };

  public:
    [[nodiscard]] static auto make(hgn::window& win) noexcept -> std::expected<hardware, error> {
        const auto app              = hgn::application::builder{}
                                          .app_name("Sui")
                                          .app_version(0, 0, 1)
                                          .engine_name("No Engine")
                                          .engine_version(0, 0, 1)
                                          .build();
        auto       instance_builder = hgn::instance::builder{}.app(app);
#ifndef NDEBUG
        instance_builder.debug(true);
#endif
        auto instance = instance_builder.build();
        if (not instance) return make_error(runtime_error, "Failed to make instance.");

        auto surface = win.make_surface(*instance);
        if (not surface) return make_error(runtime_error, "Failed to make surface.");

        auto physicals = instance->enumerate_physical();
        if (not physicals)
            return make_error(runtime_error, "Failed to enumerate physical devices.");

        auto suitable_physicals =
            *physicals |
            std::views::filter([&surface](hgn::physical_device& physical) noexcept -> bool {
                return find_queue_families(physical, *surface).is_complete();
            });
        if (suitable_physicals.empty())
            return make_error(runtime_error, "Failed to find suitable physical device.");

        auto       physical = std::move(suitable_physicals.front());
        const auto indices  = find_queue_families(physical, *surface);

        auto device_queue =
            hgn::device_queue::builder{}.queue_family_index(*indices.graphics_family).build();

        auto device = hgn::device::builder{}
                          .next(hgn::physical_device_dynamic_rendering_features{})
                          .features(hgn::physical_device_features{})
                          .queue(device_queue)
                          .build(physical);
        if (not device) return make_error(runtime_error, "Failed to make device.");

        auto graphics_queue = device->get_queue(*indices.graphics_family);
        auto present_queue  = device->get_queue(*indices.present_family);

        auto sem1  = device->make_semaphore();
        auto sem2  = device->make_semaphore();
        auto fence = device->make_fence();

        if (not sem1 or not sem2) return make_error(runtime_error, "Failed to make semaphore.");
        if (not fence) return make_error(runtime_error, "Failed to make fence.");

        return hardware{
            std::move(*instance),
            std::move(*surface),
            std::move(physical),
            std::move(*device),
            std::move(graphics_queue),
            std::move(present_queue),
            std::move(*sem1),
            std::move(*sem2),
            std::move(*fence)
        };
    }

    [[nodiscard]] auto surface() noexcept -> hgn::surface_khr& { return surface_; }
    [[nodiscard]] auto physical() noexcept -> hgn::physical_device& { return physical_; }
    [[nodiscard]] auto device() noexcept -> hgn::device& { return device_; }
    [[nodiscard]] auto graphics_queue() noexcept -> hgn::queue& { return graphics_queue_; }
    [[nodiscard]] auto present_queue() noexcept -> hgn::queue& { return present_queue_; }
    [[nodiscard]] auto image_available_semaphore() noexcept -> hgn::semaphore& {
        return image_available_;
    }
    [[nodiscard]] auto render_finished_semaphore() noexcept -> hgn::semaphore& {
        return render_finished_;
    }
    [[nodiscard]] auto in_flight_fence() noexcept -> hgn::fence& { return in_flight_; }

  private:
    explicit hardware(
        hgn::instance&&        instance,
        hgn::surface_khr&&     surface,
        hgn::physical_device&& physical,
        hgn::device&&          device,
        hgn::queue&&           graphics,
        hgn::queue&&           present,
        hgn::semaphore&&       image_available,
        hgn::semaphore&&       render_finished,
        hgn::fence&&           in_flight
    ) noexcept
        : instance_{std::move(instance)},
          surface_{std::move(surface)},
          physical_{std::move(physical)},
          device_{std::move(device)},
          graphics_queue_{std::move(graphics)},
          present_queue_{std::move(present)},
          image_available_{std::move(image_available)},
          render_finished_{std::move(render_finished)},
          in_flight_{std::move(in_flight)} {}

    [[nodiscard]] static auto find_queue_families(
        hgn::physical_device& physical, hgn::surface_khr& surface
    ) noexcept -> queue_family_indices {
        auto indices = queue_family_indices{};

        auto queue_families = physical.get_queue_family_properties();

        for (const auto [i, queue_family] : std::views::enumerate(queue_families)) {
            if ((queue_family.flags() & hgn::queue_flag_bits::graphics) !=
                hgn::queue_flag_bits::none) {
                indices.graphics_family = i;
            }

            const auto supporting =
                physical.is_supporting_surface_khr(static_cast<std::uint32_t>(i), surface);
            if (supporting and *supporting) {
                indices.present_family = i;
            }

            if (indices.is_complete()) break;
        }

        return indices;
    }

    hgn::instance        instance_;
    hgn::surface_khr     surface_;
    hgn::physical_device physical_;
    hgn::device          device_;
    hgn::queue           graphics_queue_;
    hgn::queue           present_queue_;
    hgn::semaphore       image_available_;
    hgn::semaphore       render_finished_;
    hgn::fence           in_flight_;
};
}  // namespace sui