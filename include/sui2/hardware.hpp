#pragma once

#include <expected>
#include <optional>
#include <ranges>

#include <hgn2/application.hpp>
#include <hgn2/instance.hpp>
#include <hgn2/physical.hpp>
#include <hgn2/surface.hpp>
#include <hgn2/window.hpp>

#include "sui2/error.hpp"

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

        auto physical = std::move(suitable_physicals.front());

        return hardware{std::move(*instance)};
    }

  private:
    explicit hardware(
        hgn::instance&& instance, hgn::surface_khr&& surface, hgn::physical_device&& physical
    ) noexcept;

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
                physical.is_surface_supporting_khr(static_cast<std::uint32_t>(i), surface);
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
};
}  // namespace sui