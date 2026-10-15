#pragma once

#include <array>
#include <cassert>
#include <expected>
#include <limits>
#include <optional>
#include <ranges>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/surface.hpp>
#include <hgn/swapchain.hpp>

#include "sui/device.hpp"
#include "sui/error.hpp"
#include "sui/physical_device.hpp"
#include "sui/surface.hpp"

namespace sui {
class swapchain final {
  public:
    [[nodiscard]] static auto try_make(
        physical_device& physical, surface& sur, device& dev
    ) noexcept -> std::expected<swapchain, error> {
        const auto capabilities_res =
            hgn::try_get_physical_device_surface_capabilities_khr(physical.native(), sur.native());
        if (not capabilities_res)
            return make_error(runtime_error, "Failed to get physical device surface capabilities.");
        const auto capabilities = *capabilities_res;

        const auto formats_res =
            hgn::try_get_physical_device_surface_formats_khr(physical.native(), sur.native());
        if (not formats_res)
            return make_error(runtime_error, "Failed to get physical device surface formats.");
        const auto& formats = *formats_res;

        const auto present_modes_res =
            hgn::try_get_physical_device_surface_present_modes_khr(physical.native(), sur.native());
        if (not present_modes_res)
            return make_error(
                runtime_error, "Failed to get physical device surface present modes."
            );
        const auto& present_modes = *present_modes_res;

        if (formats.empty()) return make_error(runtime_error, "Format size is required positive.");
        auto surface_format = formats.front();
        auto aligned_formats =
            formats |
            std::views::filter([](const hgn::surface_format_khr available_format) noexcept -> bool {
                return available_format.setting_format() == hgn::format::b8g8r8a8_srgb and
                       available_format.color_space() == hgn::color_space_khr::srgb_nonlinear;
            });
        if (not aligned_formats.empty()) surface_format = aligned_formats.front();

        auto present_mode = hgn::present_mode_khr::fifo;
        auto aligned_modes =
            present_modes |
            std::views::filter([](const hgn::present_mode_khr available_mode) noexcept -> bool {
                return available_mode == hgn::present_mode_khr::mailbox;
            });
        if (not aligned_modes.empty()) present_mode = aligned_modes.front();

        auto extent = capabilities.current_extent();
        if (extent.width == std::numeric_limits<std::uint32_t>::max()) {
            extent = hgn::extent2d{.width = 800, .height = 600};
        }

        auto image_count = capabilities.min_image_count() + 1;
        if (capabilities.max_image_count() > 0 and image_count > capabilities.max_image_count()) {
            image_count = capabilities.max_image_count();
        }

        auto info = hgn::swapchain_create_info_khr{};
        info.surface(sur.native())
            .min_image_count(image_count)
            .image_format(surface_format.setting_format())
            .image_color_space(surface_format.color_space())
            .image_extent(extent)
            .image_array_layers(1)
            .image_usage(hgn::image_usage_flags::color_attachment);

        auto indices = std::array{physical.graphics_family(), physical.present_family()};

        if (physical.graphics_family() != physical.present_family()) {
            info.image_sharing_mode(hgn::sharing_mode::concurrent).queue_family_indices(indices);
        } else {
            info.image_sharing_mode(hgn::sharing_mode::exclusive);
        }

        info.pre_transform(capabilities.current_transform())
            .composite_alpha(hgn::composite_alpha_flag_bits_khr::opaque)
            .present_mode(present_mode)
            .clipped(true);

        auto res = hgn::try_make_swapchain(dev.native(), info);
        if (not res) return make_error(runtime_error, "Failed to make swapchain");

        return swapchain{res->get(), dev.native(), surface_format.setting_format()};
    }

    explicit swapchain(
        hgn::swapchain& native, hgn::device& device, const hgn::format setting_format
    )
        : native_{native}, device_{device}, setting_format_{setting_format} {}

    swapchain(const swapchain&) noexcept = delete;

    auto operator=(const swapchain&) noexcept -> swapchain& = delete;

    swapchain(swapchain&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)},
          device_{std::exchange(other.device_, std::nullopt)},
          setting_format_{other.setting_format_} {}

    auto operator=(swapchain&& other) noexcept -> swapchain& {
        if (this == &other) return *this;

        destroy();

        native_         = std::exchange(other.native_, std::nullopt);
        device_         = std::exchange(other.device_, std::nullopt);
        setting_format_ = other.setting_format_;

        return *this;
    }

    ~swapchain() noexcept = default;

    [[nodiscard]] auto setting_format() const noexcept -> hgn::format { return setting_format_; }

    [[nodiscard]] auto native() noexcept -> hgn::swapchain& { return native_.value(); }

  private:
    void destroy() noexcept {
        if (not native_) return;
        assert(device_);
        hgn::destroy_swapchain_khr(*device_, *native_);
    }

    std::optional<hgn::swapchain&> native_;
    std::optional<hgn::device&>    device_;
    hgn::format                    setting_format_;
};
}  // namespace sui