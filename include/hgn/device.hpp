#pragma once

#include <vulkan/vulkan.h>
#include <cassert>
#include <expected>
#include <functional>
#include <optional>

#include "hgn/detail/vector.hpp"
#include "hgn/detail/wrapper.hpp"
#include "hgn/instance.hpp"
#include "hgn/others.hpp"
#include "hgn/result.hpp"
#include "hgn/surface.hpp"
#include "hgn/types.hpp"

namespace hgn {
using physical_device = VkPhysicalDevice_T;

template <>
class proxy_vector<std::optional<physical_device&>>
    : public detail::pointer_vector<physical_device*> {
    using pointer_vector<physical_device*>::pointer_vector;
};

[[nodiscard]] inline auto try_enumerate_physical_devices(instance& instance_ref) noexcept
    -> std::expected<proxy_vector<std::optional<physical_device&>>, result> {
    auto count = u32{};

    const auto res1 = vkEnumeratePhysicalDevices(&instance_ref, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto devices = proxy_vector<std::optional<physical_device&>>(count);

    const auto res2 = vkEnumeratePhysicalDevices(&instance_ref, &count, devices.native_data());
    if (res2 != VK_SUCCESS) return make_result(res2);

    return devices;
}

class physical_device_properties : public detail::wrapper<VkPhysicalDeviceProperties> {
  public:
    using wrapper<VkPhysicalDeviceProperties>::wrapper;

    [[nodiscard]] auto name() const noexcept -> std::string_view {
        const auto* ptr = static_cast<const char*>(native().deviceName);
        return std::string_view{ptr};
    }
};

[[nodiscard]] inline auto get_physical_device_properties(physical_device& device) noexcept
    -> physical_device_properties {
    auto properties = physical_device_properties{};
    vkGetPhysicalDeviceProperties(&device, &properties.native());
    return properties;
}

class queue_family_properties : public detail::wrapper<VkQueueFamilyProperties> {
  public:
    using wrapper<VkQueueFamilyProperties>::wrapper;

    [[nodiscard]] auto flags() const noexcept -> queue_flag_bits {
        return static_cast<queue_flag_bits>(native().queueFlags);
    }
};

template <>
class proxy_vector<queue_family_properties> final
    : public detail::view_vector<VkQueueFamilyProperties, queue_family_properties> {
    using view_vector<VkQueueFamilyProperties, queue_family_properties>::view_vector;
};

[[nodiscard]] inline auto get_physical_device_queue_family_properties(
    physical_device& device
) noexcept -> proxy_vector<queue_family_properties> {
    auto count = u32{};
    vkGetPhysicalDeviceQueueFamilyProperties(&device, &count, nullptr);

    auto queue_families = proxy_vector<queue_family_properties>(count);
    vkGetPhysicalDeviceQueueFamilyProperties(&device, &count, queue_families.native_data());

    return queue_families;
}

class device_queue_create_info final : public detail::wrapper<VkDeviceQueueCreateInfo> {
  public:
    explicit device_queue_create_info() noexcept {
        native().sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    }

    using wrapper<VkDeviceQueueCreateInfo>::wrapper;

    auto queue_family_index(const u32 idx) noexcept -> device_queue_create_info& {
        native().queueFamilyIndex = idx;
        return *this;
    }

    auto queue_count(const u32 cnt) noexcept -> device_queue_create_info& {
        native().queueCount = cnt;
        return *this;
    }

    auto queue_priorities(f32& priority) noexcept -> device_queue_create_info& {
        native().pQueuePriorities = &priority;
        return *this;
    }
};

template <>
class proxy_vector<device_queue_create_info> final
    : public detail::view_vector<VkDeviceQueueCreateInfo, device_queue_create_info> {
    using view_vector<VkDeviceQueueCreateInfo, device_queue_create_info>::view_vector;
};

using physical_device_features = VkPhysicalDeviceFeatures;

class device_create_info final : public detail::wrapper<VkDeviceCreateInfo> {
  public:
    explicit device_create_info() noexcept {
        native().sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    }

    auto queue_create_infos(const proxy_vector<device_queue_create_info>& infos) noexcept
        -> device_create_info& {
        native().queueCreateInfoCount = static_cast<u32>(infos.size());
        native().pQueueCreateInfos    = infos.native_data();
        return *this;
    }

    auto features(physical_device_features& features) noexcept -> device_create_info& {
        native().pEnabledFeatures = &features;
        return *this;
    }

    auto extensions(std::span<const char* const> vec) noexcept -> device_create_info& {
        native().enabledExtensionCount   = static_cast<u32>(vec.size());
        native().ppEnabledExtensionNames = vec.data();
        return *this;
    }
};

using device = VkDevice_T;

[[nodiscard]] inline auto try_make_device(
    physical_device& physical, const device_create_info& info
) noexcept -> std::expected<ref_w<device>, result> {
    auto*      dev = static_cast<device*>(nullptr);
    const auto res = vkCreateDevice(&physical, &info.native(), nullptr, &dev);
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

class surface_capabilities_khr : public detail::wrapper<VkSurfaceCapabilitiesKHR> {
  public:
    using wrapper<VkSurfaceCapabilitiesKHR>::wrapper;

    [[nodiscard]] auto current_extent() const noexcept -> extent2d {
        return native().currentExtent;
    }

    [[nodiscard]] auto min_image_count() const noexcept -> u32 { return native().minImageCount; }

    [[nodiscard]] auto max_image_count() const noexcept -> u32 { return native().maxImageCount; }

    [[nodiscard]] auto current_transform() const noexcept -> surface_transform_flag_bits_khr {
        return static_cast<surface_transform_flag_bits_khr>(native().currentTransform);
    }
};

[[nodiscard]] inline auto try_get_physical_device_surface_capabilities_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> std::expected<surface_capabilities_khr, result> {
    auto capabilities = surface_capabilities_khr{};  // NOLINT
    auto res =
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(&physical, &surface, &capabilities.native());
    if (res != VK_SUCCESS) return make_result(res);
    return capabilities;
}
class surface_format_khr final : public detail::wrapper<VkSurfaceFormatKHR> {
  public:
    using wrapper<VkSurfaceFormatKHR>::wrapper;

    [[nodiscard]] auto setting_format() const noexcept -> format {
        return static_cast<format>(native().format);
    }

    [[nodiscard]] auto color_space() const noexcept -> color_space_khr {
        return static_cast<color_space_khr>(native().colorSpace);
    }
};

template <>
class proxy_vector<surface_format_khr>
    : public detail::view_vector<VkSurfaceFormatKHR, surface_format_khr> {
    using view_vector<VkSurfaceFormatKHR, surface_format_khr>::view_vector;
};

[[nodiscard]] inline auto get_physical_device_surface_formats_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> proxy_vector<surface_format_khr> {
    auto count = u32{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(&physical, &surface, &count, nullptr);

    auto formats = proxy_vector<surface_format_khr>(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(&physical, &surface, &count, formats.native_data());
    return formats;
}

class present_mode_khr {
  public:
    explicit present_mode_khr(VkPresentModeKHR& native) noexcept : native_{native} {}

    auto operator=(present_mode_khr_t scoped) noexcept -> present_mode_khr& {
        native_ = static_cast<VkPresentModeKHR>(scoped);
        return *this;
    }

    [[nodiscard]] auto operator==(const present_mode_khr_t scoped) noexcept -> bool {
        return static_cast<VkPresentModeKHR>(scoped) == native_;
    }

    [[nodiscard]] operator present_mode_khr_t() const noexcept {
        return static_cast<present_mode_khr_t>(native_);
    }

    [[nodiscard]] auto native() noexcept -> VkPresentModeKHR& { return native_; }

  private:
    VkPresentModeKHR& native_;
};

template <>
class proxy_vector<present_mode_khr>
    : public detail::view_vector<VkPresentModeKHR, present_mode_khr> {
    using view_vector<VkPresentModeKHR, present_mode_khr>::view_vector;
};

[[nodiscard]] inline auto get_physical_device_surface_present_modes_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> std::expected<proxy_vector<present_mode_khr>, result> {
    auto       count = u32{};
    const auto res1 =
        vkGetPhysicalDeviceSurfacePresentModesKHR(&physical, &surface, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto       modes = proxy_vector<present_mode_khr>(count);
    const auto res2 =
        vkGetPhysicalDeviceSurfacePresentModesKHR(&physical, &surface, &count, modes.native_data());
    if (res2 != VK_SUCCESS) return make_result(res2);
    return modes;
}
}  // namespace hgn