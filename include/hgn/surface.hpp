#pragma once

#include <bit>
#include <cstdint>
#include <expected>
#include <ranges>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "hgn/enums.hpp"
#include "hgn/instance.hpp"
#include "hgn/physical_device.hpp"

namespace hgn {
using surface_khr = VkSurfaceKHR_T;

inline void destroy_surface_khr(instance& ins, surface_khr& target) noexcept {
    vkDestroySurfaceKHR(&ins, &target, nullptr);
}

[[nodiscard]] inline auto is_physical_device_surface_support_khr(
    physical_device& physical, const std::uint32_t indices, surface_khr& surface
) noexcept -> bool {
    auto present_support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(&physical, indices, &surface, &present_support);
    return present_support == VK_TRUE;
}

class surface_capabilities_khr final {
  public:
    explicit surface_capabilities_khr() noexcept = default;

    [[nodiscard]] auto current_extent() const noexcept -> extent2d { return native_.currentExtent; }

    [[nodiscard]] auto min_image_count() const noexcept -> std::uint32_t {
        return native_.minImageCount;
    }

    [[nodiscard]] auto max_image_count() const noexcept -> std::uint32_t {
        return native_.maxImageCount;
    }

    [[nodiscard]] auto current_transform() const noexcept -> surface_transform_flag_bits_khr {
        return static_cast<surface_transform_flag_bits_khr>(native_.currentTransform);
    }

  private:
    VkSurfaceCapabilitiesKHR native_;
};

[[nodiscard]] inline auto try_get_physical_device_surface_capabilities_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> std::expected<surface_capabilities_khr, result> {
    auto capabilities = VkSurfaceCapabilitiesKHR{
        .minImageCount           = 0,
        .maxImageCount           = 0,
        .currentExtent           = {},
        .minImageExtent          = {},
        .maxImageExtent          = {},
        .maxImageArrayLayers     = 0,
        .supportedTransforms     = 0,
        .currentTransform        = static_cast<VkSurfaceTransformFlagBitsKHR>(0),
        .supportedCompositeAlpha = 0,
        .supportedUsageFlags     = 0
    };

    auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(&physical, &surface, &capabilities);
    if (res != VK_SUCCESS) return make_result(res);

    return std::bit_cast<surface_capabilities_khr>(capabilities);
}

class surface_format_khr final {
  public:
    explicit surface_format_khr() noexcept = default;

    [[nodiscard]] auto setting_format() const noexcept -> format {
        return static_cast<format>(native_.format);
    }

    [[nodiscard]] auto color_space() const noexcept -> color_space_khr {
        return static_cast<color_space_khr>(native_.colorSpace);
    }

  private:
    VkSurfaceFormatKHR native_{};
};

[[nodiscard]] inline auto get_physical_device_surface_formats_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> std::vector<surface_format_khr> {
    auto count = std::uint32_t{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(&physical, &surface, &count, nullptr);

    auto formats = std::vector<VkSurfaceFormatKHR>(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(&physical, &surface, &count, formats.data());

    auto out = std::vector<surface_format_khr>{};
    out.reserve(count);
    out.assign_range(
        formats |
        std::views::transform([](const VkSurfaceFormatKHR& format) noexcept -> surface_format_khr {
            return std::bit_cast<surface_format_khr>(format);
        })
    );

    return out;
}

[[nodiscard]] inline auto get_physical_device_surface_present_modes_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> std::expected<std::vector<present_mode_khr>, result> {
    auto       count = std::uint32_t{};
    const auto res1 =
        vkGetPhysicalDeviceSurfacePresentModesKHR(&physical, &surface, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto       modes = std::vector<VkPresentModeKHR>(count);
    const auto res2 =
        vkGetPhysicalDeviceSurfacePresentModesKHR(&physical, &surface, &count, modes.data());
    if (res2 != VK_SUCCESS) return make_result(res2);

    auto out = std::vector<present_mode_khr>{};
    out.reserve(count);
    out.assign_range(
        modes | std::views::transform([](const VkPresentModeKHR mode) noexcept -> present_mode_khr {
            return std::bit_cast<present_mode_khr>(mode);
        })
    );

    return out;
}
}  // namespace hgn