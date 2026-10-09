#pragma once

#include <vulkan/vulkan.h>
#include <cassert>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "hgn/detail/vector.hpp"
#include "hgn/instance.hpp"
#include "hgn/others.hpp"
#include "hgn/result.hpp"
#include "hgn/surface.hpp"
#include "hgn/types.hpp"

namespace hgn {
using physical_device = VkPhysicalDevice_T;

template <>
class vector<std::optional<physical_device&>> : public detail::base_vector<physical_device*> {
    using base_vector<physical_device*>::base_vector;
};

[[nodiscard]] inline auto try_enumerate_physical_devices(instance& instance_ref) noexcept
    -> std::expected<vector<std::optional<physical_device&>>, result> {
    auto count = u32{};

    const auto res1 = vkEnumeratePhysicalDevices(&instance_ref, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto devices = vector<std::optional<physical_device&>>(count);

    const auto res2 = vkEnumeratePhysicalDevices(&instance_ref, &count, devices.native_data());
    if (res2 != VK_SUCCESS) return make_result(res2);

    return devices;
}

using physical_device_properties = VkPhysicalDeviceProperties;

[[nodiscard]] inline auto get_physical_device_properties(physical_device& device) noexcept
    -> physical_device_properties {
    auto properties = physical_device_properties{};
    vkGetPhysicalDeviceProperties(&device, &properties);
    return properties;
}

class physical_device_properties_view final {
  public:
    explicit physical_device_properties_view(const physical_device_properties& properties) noexcept
        : prop_{properties} {}

    [[nodiscard]] auto device_name() const noexcept -> std::string {
        return prop_.deviceName;  // NOLINT
    }

  private:
    const physical_device_properties& prop_;
};

class queue_family_properties : public detail::wrapper<VkQueueFamilyProperties> {
  public:
    using wrapper<VkQueueFamilyProperties>::wrapper;

    [[nodiscard]] auto flags() const noexcept -> queue_flag_bits {
        return static_cast<queue_flag_bits>(native().queueFlags);
    }
};

template <>
class vector<queue_family_properties> final
    : public detail::base_vector<VkQueueFamilyProperties, queue_family_properties> {
    using base_vector<VkQueueFamilyProperties, queue_family_properties>::base_vector;
};

[[nodiscard]] inline auto get_physical_device_queue_family_properties(
    physical_device& device
) noexcept -> vector<queue_family_properties> {
    auto count = u32{};
    vkGetPhysicalDeviceQueueFamilyProperties(&device, &count, nullptr);

    auto queue_families = vector<queue_family_properties>(count);
    vkGetPhysicalDeviceQueueFamilyProperties(&device, &count, queue_families.native_data());

    return queue_families;
}

using device_queue_create_info = VkDeviceQueueCreateInfo;

class device_queue_create_info_setter final {
  public:
    explicit device_queue_create_info_setter(device_queue_create_info& info) noexcept
        : info_{info} {
        info_.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    }

    auto queue_family_index(const u32 idx) noexcept -> device_queue_create_info_setter& {
        info_.queueFamilyIndex = idx;
        return *this;
    }

    auto queue_count(const u32 cnt) noexcept -> device_queue_create_info_setter& {
        info_.queueCount = cnt;
        return *this;
    }

    auto queue_priorities(f32& priority) noexcept -> device_queue_create_info_setter& {
        info_.pQueuePriorities = &priority;
        return *this;
    }

  private:
    device_queue_create_info& info_;
};

using physical_device_features = VkPhysicalDeviceFeatures;
using device_create_info       = VkDeviceCreateInfo;

class device_create_info_setter final {
  public:
    explicit device_create_info_setter(device_create_info& info) : info_{info} {
        info_.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    }

    auto queue_create_infos(const std::span<const device_queue_create_info> infos) noexcept
        -> device_create_info_setter& {
        info_.queueCreateInfoCount = static_cast<u32>(infos.size());
        info_.pQueueCreateInfos    = infos.data();
        return *this;
    }

    auto features(physical_device_features& features) noexcept -> device_create_info_setter& {
        info_.pEnabledFeatures = &features;
        return *this;
    }

    auto extensions(std::span<const char* const> vec) noexcept -> device_create_info_setter& {
        info_.enabledExtensionCount   = static_cast<u32>(vec.size());
        info_.ppEnabledExtensionNames = vec.data();
        return *this;
    }

  private:
    device_create_info& info_;
};

using device = VkDevice_T;

[[nodiscard]] inline auto try_make_device(
    physical_device& physical, const device_create_info& info
) noexcept -> std::expected<ref_w<device>, result> {
    auto*      dev = static_cast<device*>(nullptr);
    const auto res = vkCreateDevice(&physical, &info, nullptr, &dev);
    if (res != VK_SUCCESS) return make_result(res);

    assert(dev != nullptr);
    return std::ref(*dev);
}

inline void destroy_device(device& dev) noexcept {
    vkDestroyDevice(&dev, nullptr);
}

using queue = VkQueue_T;

[[nodiscard]] inline auto try_get_device_queue(device& dev, const u32 graphic_family) noexcept
    -> std::optional<queue&> {
    auto* out = static_cast<queue*>(nullptr);
    vkGetDeviceQueue(&dev, graphic_family, 0, &out);
    if (out == nullptr) return std::nullopt;
    return *out;
}

[[nodiscard]] inline auto is_physical_device_surface_support_khr(
    physical_device& physical, u32 indices, surface_khr& surface
) noexcept -> bool {
    auto present_support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(&physical, indices, &surface, &present_support);
    return present_support == VK_TRUE;
}

using surface_capabilities_khr = VkSurfaceCapabilitiesKHR;

[[nodiscard]] inline auto try_get_physical_device_surface_capabilities_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> std::expected<surface_capabilities_khr, result> {
    auto capabilities = surface_capabilities_khr{};  // NOLINT
    auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(&physical, &surface, &capabilities);
    if (res != VK_SUCCESS) return make_result(res);
    return capabilities;
}

using surface_format_khr = VkSurfaceFormatKHR;

[[nodiscard]] inline auto get_physical_device_surface_formats_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> std::vector<surface_format_khr> {
    auto count = u32{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(&physical, &surface, &count, nullptr);

    auto formats = std::vector<surface_format_khr>(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(&physical, &surface, &count, formats.data());
    return formats;
}

using present_mode_khr = VkPresentModeKHR;

[[nodiscard]] inline auto get_physical_device_surface_present_modes_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> std::expected<std::vector<present_mode_khr>, result> {
    auto       count = u32{};
    const auto res1 =
        vkGetPhysicalDeviceSurfacePresentModesKHR(&physical, &surface, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto       modes = std::vector<present_mode_khr>(count);
    const auto res2 =
        vkGetPhysicalDeviceSurfacePresentModesKHR(&physical, &surface, &count, modes.data());
    if (res2 != VK_SUCCESS) return make_result(res2);
    return modes;
}
}  // namespace hgn