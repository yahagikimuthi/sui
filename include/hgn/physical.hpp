#pragma once

#include <bit>
#include <cstdint>
#include <expected>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw.hpp"
#include "hgn/structs.hpp"
#include "hgn/surface.hpp"

namespace hgn {
class physical_device final {
    friend class instance;

  public:
    physical_device(const physical_device&) noexcept                    = delete;
    auto operator=(const physical_device&) noexcept -> physical_device& = delete;
    physical_device(physical_device&&) noexcept                         = default;
    auto operator=(physical_device&&) noexcept -> physical_device&      = default;
    ~physical_device() noexcept                                         = default;

    [[nodiscard]] auto name() const noexcept -> std::string {
        const auto property = get_property();
        return std::string{static_cast<const char*>(property.deviceName)};
    }

    [[nodiscard]] auto get_queue_family_properties() noexcept
        -> std::vector<queue_family_properties> {
        auto count = std::uint32_t{};
        vkGetPhysicalDeviceQueueFamilyProperties(&get(), &count, nullptr);

        auto properties = std::vector<VkQueueFamilyProperties>(count);
        vkGetPhysicalDeviceQueueFamilyProperties(&get(), &count, properties.data());

        return properties |
               std::views::transform(
                   [](const VkQueueFamilyProperties& property) noexcept -> queue_family_properties {
                       return std::bit_cast<queue_family_properties>(property);
                   }
               ) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto is_supporting_surface_khr(
        const std::uint32_t queue_family_index, surface_khr& surface
    ) noexcept -> std::expected<bool, result> {
        auto       supported = VkBool32{};
        const auto res       = vkGetPhysicalDeviceSurfaceSupportKHR(
            &get(), queue_family_index, &surface.get(), &supported
        );
        if (res != VK_SUCCESS) return make_unexpected(res);

        return supported != VK_FALSE;
    }

    [[nodiscard]] auto get_surface_capabilities_khr(surface_khr& surface) noexcept
        -> std::expected<surface_capabilities_khr, result> {
        auto       capabilities = VkSurfaceCapabilitiesKHR{};  // NOLINT
        const auto res =
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(&get(), &surface.get(), &capabilities);
        if (res != VK_SUCCESS) return make_unexpected(res);

        return std::bit_cast<surface_capabilities_khr>(capabilities);
    }

    [[nodiscard]] auto get_surface_formats_khr(surface_khr& surface) noexcept
        -> std::expected<std::vector<surface_format_khr>, result> {
        auto       count = std::uint32_t{};
        const auto res1 =
            vkGetPhysicalDeviceSurfaceFormatsKHR(&get(), &surface.get(), &count, nullptr);
        if (res1 != VK_SUCCESS) return make_unexpected(res1);

        auto       formats = std::vector<VkSurfaceFormatKHR>(count);
        const auto res2 =
            vkGetPhysicalDeviceSurfaceFormatsKHR(&get(), &surface.get(), &count, formats.data());
        if (res2 != VK_SUCCESS) return make_unexpected(res2);

        return formats |
               std::views::transform(
                   [](const VkSurfaceFormatKHR& raw) noexcept -> surface_format_khr {
                       return std::bit_cast<surface_format_khr>(raw);
                   }
               ) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto get_surface_present_modes_khr(surface_khr& surface) noexcept
        -> std::expected<std::vector<present_mode_khr>, result> {
        auto       count = std::uint32_t{};
        const auto res1 =
            vkGetPhysicalDeviceSurfacePresentModesKHR(&get(), &surface.get(), &count, nullptr);
        if (res1 != VK_SUCCESS) return make_unexpected(res1);

        auto       modes = std::vector<VkPresentModeKHR>(count);
        const auto res2 =
            vkGetPhysicalDeviceSurfacePresentModesKHR(&get(), &surface.get(), &count, modes.data());
        if (res2 != VK_SUCCESS) return make_unexpected(res2);

        return modes |
               std::views::transform([](VkPresentModeKHR& mode) noexcept -> present_mode_khr {
                   return static_cast<present_mode_khr>(mode);
               }) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto get_memory_properties() noexcept -> physical_device_memory_properties {
        auto property = VkPhysicalDeviceMemoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(&get(), &property);
        return std::bit_cast<physical_device_memory_properties>(property);
    }

    [[nodiscard]] auto get() noexcept -> raw::physical_device& { return raw_; }

    [[nodiscard]] auto get() const noexcept -> raw::physical_device& { return raw_; }

  private:
    explicit physical_device(raw::physical_device& raw) noexcept : raw_{std::move(raw)} {}

    [[nodiscard]] auto get_property() const noexcept -> VkPhysicalDeviceProperties {
        auto property = VkPhysicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(&get(), &property);
        return property;
    }

    raii::physical_device raw_;
};
}  // namespace hgn