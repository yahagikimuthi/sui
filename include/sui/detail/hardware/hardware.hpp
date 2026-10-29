#pragma once

#include <expected>
#include <ranges>
#include <utility>

#include <hgn/enums.hpp>
#include <kgm/kgm.hpp>
#include <kgm/window.hpp>

#include "hgn_oop/hardware.hpp"
#include "sui/detail/hardware/physical_device.hpp"
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
        const auto app_info = hgn::application_info{}
                                  .application_name("sui")
                                  .application_version(1, 0, 0)
                                  .engine_name("No Engine")
                                  .engine_version(1, 0, 0)
                                  .api_version(1, 3, 0);

        auto instance_info =
            hgn::instance_create_info{}.app_info(app_info).extensions(required_extensions());

#ifndef NDEBUG
        instance_info.layer("VK_LAYER_KHRONOS_validation");
#endif
        auto instance = hgn::oop::try_make_instance(instance_info);
        if (not instance) return make_error(runtime_error, "Failed to make instance.");

        auto native_surface = kgm::try_make_window_surface(instance->get(), win);
        if (not native_surface) return make_error(runtime_error, "Failed to make surface.");
        auto surface = hgn::oop::make_surface(std::move(native_surface->get()), *instance);

        auto physicals = hgn::oop::try_enumerate_physical(*instance);
        if (not physicals)
            return make_error(runtime_error, "Failed to enumerate physical devices.");

        auto suitable_physicals =
            *physicals |
            std::views::filter([&](hgn::oop::physical_device& physical) noexcept -> bool {
                return is_device_suitable(physical, surface);
            });

        if (suitable_physicals.empty())
            return make_error(runtime_error, "Failed to find a suitable GPU.");

        const auto indices = find_queue_families(suitable_physicals.front(), surface);

        auto& native_physical = suitable_physicals.front();
        auto  physical =
            physical_device{native_physical, *indices.graphics_family, *indices.present_family};

        const auto queue_info =
            hgn::device_queue_create_info{}.queue_family_index(physical.graphics_family());

        const auto dev_info =
            hgn::device_create_info{}
                .queue_create_info(queue_info)
                .features(hgn::physical_device_features{})
                .extensions({hgn::khr_swapchain_extention_name})
                .dynamic_rendering_features(hgn::physical_device_dynamic_rendering_features{});

        auto dev = hgn::oop::try_make_device(dev_info, native_physical);
        if (not dev) return make_error(runtime_error, "Failed to make device.");

        auto graphics_queue = hgn::oop::try_make_queue(*dev, physical.graphics_family());
        if (not graphics_queue) return make_error(runtime_error, "Failed to make graphics queue.");

        auto present_queue = hgn::oop::try_make_queue(*dev, physical.present_family());
        if (not present_queue) return make_error(runtime_error, "Failed to make present queue.");

        return hardware{
            std::move(*instance),
            std::move(surface),
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

    [[nodiscard]] auto get_surface() noexcept -> hgn::oop::surface_khr& { return surface_; }
    [[nodiscard]] auto get_physical() noexcept -> physical_device& { return physical_; }
    [[nodiscard]] auto get_device() noexcept -> hgn::oop::device& { return device_; }
    [[nodiscard]] auto get_graphics_queue() noexcept -> hgn::oop::queue& { return graphics_; }
    [[nodiscard]] auto get_present_queue() noexcept -> hgn::oop::queue& { return present_; }

  private:
    explicit hardware(
        hgn::oop::instance&&    inst,
        hgn::oop::surface_khr&& sur,
        const physical_device&  physical,
        hgn::oop::device&&      dev,
        const hgn::oop::queue&  graphics,
        const hgn::oop::queue&  present
    ) noexcept
        : instance_{std::move(inst)},
          surface_{std::move(sur)},
          physical_{physical},
          device_{std::move(dev)},
          graphics_{graphics},
          present_{present} {}

    [[nodiscard]] static auto required_extensions() noexcept -> std::vector<std::string_view> {
        auto extensions = kgm::get_required_instance_extensions();

#ifndef NDEBUG
        extensions.emplace_back(hgn::ext_debug_utils_extension_name);
#endif
        return extensions;
    }

    [[nodiscard]] static auto find_queue_families(
        hgn::oop::physical_device& physical, hgn::oop::surface_khr& surface
    ) noexcept -> queue_family_indices {
        auto indices = queue_family_indices{};

        auto queue_families = physical.get_queue_family_properties();

        for (const auto [i, queue_family] : std::views::enumerate(queue_families)) {
            if ((queue_family.flags() & hgn::queue_flag_bits::graphics) !=
                hgn::queue_flag_bits::none) {
                indices.graphics_family = i;
            }

            if (physical.is_surface_supporting_khr(static_cast<std::uint32_t>(i), surface)) {
                indices.present_family = i;
            }

            if (indices.is_complete()) break;
        }

        return indices;
    }

    [[nodiscard]] static auto is_device_suitable(
        hgn::oop::physical_device& device, hgn::oop::surface_khr& surface
    ) noexcept -> bool {
        return find_queue_families(device, surface).is_complete();
    }

    hgn::oop::instance    instance_;
    hgn::oop::surface_khr surface_;
    physical_device       physical_;
    hgn::oop::device      device_;
    hgn::oop::queue       graphics_;
    hgn::oop::queue       present_;
};
}  // namespace sui::detail