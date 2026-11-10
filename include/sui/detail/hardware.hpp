#pragma once

#include <expected>
#include <optional>
#include <ranges>
#include <utility>

#include <hgn/device.hpp>
#include <hgn/hgn.hpp>
#include <hgn/instance.hpp>
#include <hgn/physical.hpp>
#include <hgn/queue.hpp>
#include <hgn/surface.hpp>
#include <hgn/sync.hpp>
#include <kgm/kgm.hpp>
#include <kgm/window.hpp>

#include "sui/error.hpp"

namespace sui::detail {
class hardware final {
    struct queue_families final {
        hgn::queue_family graphics_family;
        hgn::queue_family present_family;
    };

  public:
    [[nodiscard]] static auto make(kgm::window& win) noexcept -> std::expected<hardware, error> {
        auto instance = hgn::get_instance_creator()
                            .app_name("sui")
                            .app_version(0, 0, 1)
                            .engine_name("no engine")
                            .engine_version(0, 0, 1)
                            .extensions(kgm::get_required_instance_extensions())
#ifndef NDEBUG
                            .debug(true)
#endif
                            .create();

        if (not instance) return make_error(runtime_error, "Failed to make instance.");

        auto surface = win.create_surface(*instance);
        if (not surface) return make_error(runtime_error, "Failed to make surface.");

        auto physicals = instance->get_physicals();
        if (not physicals)
            return make_error(runtime_error, "Failed to enumerate physical devices.");

        auto suitable_physicals =
            *physicals |
            std::views::filter([&surface](hgn::physical_device& physical) noexcept -> bool {
                return find_queue_families(physical, *surface).has_value();
            });
        if (suitable_physicals.empty())
            return make_error(runtime_error, "Failed to find suitable physical device.");

        auto physical = suitable_physicals.front();
        auto families = *find_queue_families(physical, *surface);

        auto device = physical.create_device(families.graphics_family);
        if (not device) return make_error(runtime_error, "Failed to make device.");

        auto graphics_queue = device->get_queue(families.graphics_family);
        auto present_queue  = device->get_queue(families.present_family);

        auto sem1  = device->create_semaphore();
        auto sem2  = device->create_semaphore();
        auto fence = device->create_fence();

        if (not sem1 or not sem2) return make_error(runtime_error, "Failed to make semaphore.");
        if (not fence) return make_error(runtime_error, "Failed to make fence.");

        return hardware{
            std::move(*instance),
            std::move(*surface),
            physical,
            std::move(*device),
            families.graphics_family,
            families.present_family,
            graphics_queue,
            present_queue,
            std::move(*sem1),
            std::move(*sem2),
            std::move(*fence)
        };
    }

    [[nodiscard]] auto instance() noexcept -> hgn::instance& { return instance_; }
    [[nodiscard]] auto surface() noexcept -> hgn::surface_khr& { return surface_; }
    [[nodiscard]] auto physical() noexcept -> hgn::physical_device& { return physical_; }
    [[nodiscard]] auto device() noexcept -> hgn::device& { return device_; }
    [[nodiscard]] auto graphics_family() noexcept -> hgn::queue_family& { return graphics_family_; }
    [[nodiscard]] auto present_family() noexcept -> hgn::queue_family& { return present_family_; }
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
        hgn::instance&&      instance,
        hgn::surface_khr&&   surface,
        hgn::physical_device physical,
        hgn::device&&        device,
        hgn::queue_family    graphics_family,
        hgn::queue_family    present_family,
        hgn::queue           graphics,
        hgn::queue           present,
        hgn::semaphore&&     image_available,
        hgn::semaphore&&     render_finished,
        hgn::fence&&         in_flight
    ) noexcept
        : instance_{std::move(instance)},
          surface_{std::move(surface)},
          physical_{physical},
          device_{std::move(device)},
          graphics_family_{graphics_family},
          present_family_{present_family},
          graphics_queue_{graphics},
          present_queue_{present},
          image_available_{std::move(image_available)},
          render_finished_{std::move(render_finished)},
          in_flight_{std::move(in_flight)} {}

    // 物理デバイスが適合するか判定するか判定し、キューファミリーを返す
    [[nodiscard]] static auto find_queue_families(
        hgn::physical_device& physical, hgn::surface_khr& surface
    ) noexcept -> std::optional<queue_families> {
        auto graphics_family = std::optional<hgn::queue_family>{};
        auto present_family  = std::optional<hgn::queue_family>{};

        // 描画が可能でsurfaceをサポートする最初の物理デバイスを選択
        for (auto& queue_family : physical.get_queue_families()) {
            // 少なくともgraphics(描画)が可能なものを選ぶ
            if ((queue_family.flags() & hgn::queue_flag_bits::graphics) !=
                hgn::queue_flag_bits::none) {
                graphics_family = queue_family;
            }

            // surfaceをサポートするか判定
            const auto supporting = queue_family.is_supporting_surface(surface);
            if (supporting and *supporting) {
                present_family = queue_family;
            }

            if (graphics_family and present_family) {
                return queue_families{
                    .graphics_family = *graphics_family, .present_family = *present_family
                };
            }
        }

        // 存在しない場合はnull
        return std::nullopt;
    }

    hgn::instance        instance_;
    hgn::surface_khr     surface_;
    hgn::physical_device physical_;
    hgn::device          device_;
    hgn::queue_family    graphics_family_;
    hgn::queue_family    present_family_;
    hgn::queue           graphics_queue_;
    hgn::queue           present_queue_;
    hgn::semaphore       image_available_;
    hgn::semaphore       render_finished_;
    hgn::fence           in_flight_;
};
}  // namespace sui::detail