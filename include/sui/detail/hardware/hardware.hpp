#pragma once

#include <expected>
#include <ranges>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/physical_device.hpp>
#include <hgn/submit.hpp>
#include <hgn/surface.hpp>
#include <hgn/sync.hpp>
#include <hgn_raii/device.hpp>
#include <hgn_raii/instance.hpp>
#include <hgn_raii/surface.hpp>
#include <kgm/kgm.hpp>
#include <kgm/window.hpp>

#include "hgn_raii/queue.hpp"
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
        auto inst = try_make_instance();
        if (not inst) return make_error(runtime_error, "Failed to make instance.");

        auto native_surface = kgm::try_make_window_surface(*inst, win);
        if (not native_surface) return make_error(runtime_error, "Failed to make surface.");
        auto surface = hgn::raii::make_surface_khr(std::move(native_surface->get()), *inst);

        auto native_physicals_res = hgn::raii::try_enumerate_physical(*inst);
        if (not native_physicals_res)
            return make_error(runtime_error, "Failed to enumerate physical devices.");
        auto& native_physical_devices = *native_physicals_res;

        auto suitable_physicals =
            native_physical_devices |
            std::views::filter([&](hgn::raii::physical_device& physical) noexcept -> bool {
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

        auto dev = hgn::raii::try_make_device(dev_info, physical.get());
        if (not dev) return make_error(runtime_error, "Failed to make device.");

        auto graphics_queue = hgn::raii::try_get_queue(dev->get(), physical.graphics_family());
        if (not graphics_queue) return make_error(runtime_error, "Failed to make graphics queue.");

        auto present_queue = hgn::raii::try_get_queue(dev->get(), physical.present_family());
        if (not present_queue) return make_error(runtime_error, "Failed to make present queue.");

        return hardware{
            std::move(*inst),
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

    [[nodiscard]] auto try_get_physical_device_surface_capabilities() noexcept
        -> std::optional<hgn::surface_capabilities_khr> {
        const auto res =
            hgn::try_get_physical_device_surface_capabilities_khr(physical_.get(), surface_.get());
        if (not res) return std::nullopt;
        return *res;
    }

    [[nodiscard]] auto try_get_physical_device_surface_formats() noexcept
        -> std::optional<std::vector<hgn::surface_format_khr>> {
        const auto res =
            hgn::try_get_physical_device_surface_formats_khr(physical_.get(), surface_.get());
        if (not res) return std::nullopt;
        return *res;
    }

    [[nodiscard]] auto try_get_physical_device_surface_present_modes() noexcept
        -> std::optional<std::vector<hgn::present_mode_khr>> {
        const auto res =
            hgn::try_get_physical_device_surface_present_modes_khr(physical_.get(), surface_.get());
        if (not res) return std::nullopt;
        return *res;
    }

    [[nodiscard]] auto get_surface() noexcept -> hgn::raii::surface_khr& { return surface_; }
    [[nodiscard]] auto get_physical() noexcept -> physical_device& { return physical_; }
    [[nodiscard]] auto get_device() noexcept -> hgn::raii::device& { return device_; }
    [[nodiscard]] auto get_graphics_queue() noexcept -> hgn::raii::queue& { return graphics_; }
    [[nodiscard]] auto get_present_queue() noexcept -> hgn::raii::queue& { return present_; }

  private:
    explicit hardware(
        hgn::raii::instance&&    inst,
        hgn::raii::surface_khr&& sur,
        const physical_device&   physical,
        hgn::raii::device&&      dev,
        const hgn::raii::queue&  graphics,
        const hgn::raii::queue&  present
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

    [[nodiscard]] static auto try_make_instance() noexcept -> std::optional<hgn::raii::instance> {
        const auto app_info = hgn::application_info{}
                                  .application_name("sui")
                                  .application_version(1, 0, 0)
                                  .engine_name("No Engine")
                                  .engine_version(1, 0, 0)
                                  .engine_version(1, 3, 0)
                                  .api_version(1, 3, 0);

        auto info =
            hgn::instance_create_info{}.app_info(app_info).extensions(required_extensions());

#ifndef NDEBUG
        info.layer("VK_LAYER_KHRONOS_validation");
#endif
        auto native = hgn::raii::try_make_instance(info);
        if (not native) return std::nullopt;

        return std::move(*native);
    }

    [[nodiscard]] static auto find_queue_families(
        hgn::physical_device& device, hgn::raii::surface_khr& surface
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
        hgn::physical_device& device, hgn::raii::surface_khr& surface
    ) noexcept -> bool {
        return find_queue_families(device, surface).is_complete();
    }

    hgn::raii::instance    instance_;
    hgn::raii::surface_khr surface_;
    physical_device        physical_;
    hgn::raii::device      device_;
    hgn::raii::queue       graphics_;
    hgn::raii::queue       present_;
};
}  // namespace sui::detail