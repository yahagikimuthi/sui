#pragma once

#include <vulkan/vulkan.h>
#include <bit>
#include <cassert>
#include <expected>
#include <functional>
#include <optional>

#include "hgn/detail/experimenta.hpp"
#include "hgn/instance.hpp"
#include "hgn/others.hpp"
#include "hgn/result.hpp"
#include "hgn/surface.hpp"
#include "hgn/types.hpp"

namespace hgn {
using physical_device = VkPhysicalDevice_T;

[[nodiscard]] inline auto try_enumerate_physical_devices(instance& instance_ref) noexcept
    -> std::expected<std::vector<ref_w<physical_device>>, result> {
    auto count = u32{};

    const auto res1 = vkEnumeratePhysicalDevices(&instance_ref, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto devices = std::vector<physical_device*>(count);

    const auto res2 = vkEnumeratePhysicalDevices(&instance_ref, &count, devices.data());
    if (res2 != VK_SUCCESS) return make_result(res2);

    auto out = std::vector<ref_w<physical_device>>{};
    out.reserve(count);

    for (auto* device : devices) {
        assert(device != nullptr);
        out.emplace_back(std::ref(*device));
    }

    return out;
}

class physical_device_properties {
  public:
    explicit physical_device_properties(const VkPhysicalDeviceProperties& native) noexcept
        : native_{native} {}

    [[nodiscard]] auto name() const noexcept -> std::string_view {
        const auto* ptr = static_cast<const char*>(native_.deviceName);
        return std::string_view{ptr};
    }

  private:
    VkPhysicalDeviceProperties native_;
};

[[nodiscard]] inline auto get_physical_device_properties(physical_device& device) noexcept
    -> physical_device_properties {
    auto properties = VkPhysicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(&device, &properties);
    return physical_device_properties{properties};
}

class queue_family_properties final {
  public:
    explicit queue_family_properties(const VkQueueFamilyProperties& native) noexcept
        : native_{native} {}

    [[nodiscard]] auto flags() const noexcept -> queue_flag_bits {
        return static_cast<queue_flag_bits>(native_.queueFlags);
    }

  private:
    VkQueueFamilyProperties native_{};
};

[[nodiscard]] inline auto get_physical_device_queue_family_properties(
    physical_device& device
) noexcept -> std::vector<queue_family_properties> {
    auto count = u32{};
    vkGetPhysicalDeviceQueueFamilyProperties(&device, &count, nullptr);

    auto queue_families = std::vector<VkQueueFamilyProperties>(count);
    vkGetPhysicalDeviceQueueFamilyProperties(&device, &count, queue_families.data());

    auto out = std::vector<queue_family_properties>{};
    out.reserve(count);

    for (const auto& family : queue_families) {
        out.emplace_back(std::bit_cast<queue_family_properties>(family));
    }

    return out;
}

class device_queue_create_info final {
  public:
    explicit device_queue_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    }

    auto queue_family_index(const u32 idx) noexcept -> device_queue_create_info& {
        native_.queueFamilyIndex = idx;
        return *this;
    }

    auto queue_count(const u32 cnt) noexcept -> device_queue_create_info& {
        native_.queueCount = cnt;
        return *this;
    }

    auto queue_priorities(f32& priority) noexcept -> device_queue_create_info& {
        native_.pQueuePriorities = &priority;
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkDeviceQueueCreateInfo& { return native_; }

  private:
    VkDeviceQueueCreateInfo native_{};
};

using physical_device_features = VkPhysicalDeviceFeatures;

class device_create_info final {
  public:
    explicit device_create_info() noexcept { native_.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; }

    auto queue_create_infos(const std::vector<device_queue_create_info>& infos) noexcept
        -> device_create_info& {
        infos_ = std::vector<VkDeviceQueueCreateInfo>{};
        infos_.reserve(infos.size());

        for (const auto& info : infos) {
            infos_.emplace_back(std::bit_cast<VkDeviceQueueCreateInfo>(info));
        }

        native_.queueCreateInfoCount = static_cast<u32>(infos_.size());
        native_.pQueueCreateInfos    = infos_.data();
        return *this;
    }

    auto features(physical_device_features& features) noexcept -> device_create_info& {
        native_.pEnabledFeatures = &features;
        return *this;
    }

    auto extensions(std::span<const char* const> vec) noexcept -> device_create_info& {
        native_.enabledExtensionCount   = static_cast<u32>(vec.size());
        native_.ppEnabledExtensionNames = vec.data();
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkDeviceCreateInfo& { return native_; }

  private:
    VkDeviceCreateInfo                   native_{};
    std::vector<VkDeviceQueueCreateInfo> infos_;
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

class surface_capabilities_khr final {
  public:
    explicit surface_capabilities_khr(const VkSurfaceCapabilitiesKHR& native) noexcept
        : native_{native} {}

    [[nodiscard]] auto current_extent() const noexcept -> extent2d { return native_.currentExtent; }

    [[nodiscard]] auto min_image_count() const noexcept -> u32 { return native_.minImageCount; }

    [[nodiscard]] auto max_image_count() const noexcept -> u32 { return native_.maxImageCount; }

    [[nodiscard]] auto current_transform() const noexcept -> surface_transform_flag_bits_khr {
        return static_cast<surface_transform_flag_bits_khr>(native_.currentTransform);
    }

  private:
    VkSurfaceCapabilitiesKHR native_{
        .minImageCount           = 0,
        .maxImageCount           = 0,
        .currentExtent           = {.width = 0, .height = 0},
        .minImageExtent          = {.width = 0, .height = 0},
        .maxImageExtent          = {.width = 0, .height = 0},
        .maxImageArrayLayers     = 0,
        .supportedTransforms     = 0,
        .currentTransform        = static_cast<VkSurfaceTransformFlagBitsKHR>(0),
        .supportedCompositeAlpha = 0,
        .supportedUsageFlags     = 0
    };
};

[[nodiscard]] inline auto try_get_physical_device_surface_capabilities_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> std::expected<surface_capabilities_khr, result> {
    auto capabilities = VkSurfaceCapabilitiesKHR{
        .minImageCount           = 0,
        .maxImageCount           = 0,
        .currentExtent           = {.width = 0, .height = 0},
        .minImageExtent          = {.width = 0, .height = 0},
        .maxImageExtent          = {.width = 0, .height = 0},
        .maxImageArrayLayers     = 0,
        .supportedTransforms     = 0,
        .currentTransform        = static_cast<VkSurfaceTransformFlagBitsKHR>(0),
        .supportedCompositeAlpha = 0,
        .supportedUsageFlags     = 0
    };

    auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(&physical, &surface, &capabilities);
    if (res != VK_SUCCESS) return make_result(res);

    return surface_capabilities_khr{capabilities};
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
    auto count = u32{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(&physical, &surface, &count, nullptr);

    auto formats = std::vector<VkSurfaceFormatKHR>(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(&physical, &surface, &count, formats.data());

    auto out = std::vector<surface_format_khr>{};
    out.reserve(count);
    for (const auto& format : formats) {
        out.emplace_back(std::bit_cast<surface_format_khr>(format));
    }

    return out;
}

[[nodiscard]] inline auto get_physical_device_surface_present_modes_khr(
    physical_device& physical, surface_khr& surface
) noexcept -> std::expected<std::vector<present_mode_khr>, result> {
    auto       count = u32{};
    const auto res1 =
        vkGetPhysicalDeviceSurfacePresentModesKHR(&physical, &surface, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto       modes = std::vector<VkPresentModeKHR>(count);
    const auto res2 =
        vkGetPhysicalDeviceSurfacePresentModesKHR(&physical, &surface, &count, modes.data());
    if (res2 != VK_SUCCESS) return make_result(res2);

    auto out = std::vector<present_mode_khr>{};
    out.reserve(count);
    for (const auto mode : modes) {
        out.emplace_back(std::bit_cast<present_mode_khr>(mode));
    }

    return out;
}
}  // namespace hgn