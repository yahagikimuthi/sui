#pragma once

#include <vulkan/vulkan.h>
#include <expected>
#include <string>
#include <vector>

#include "hgn/instance.hpp"
#include "hgn/result.hpp"
#include "hgn/types.hpp"

namespace hgn {
using physical_device = VkPhysicalDevice;

[[nodiscard]] inline auto try_enumerate_physical_devices(const instance& instance_ref) noexcept
    -> std::expected<std::vector<physical_device>, result> {
    auto devices = std::vector<physical_device>{};
    auto count   = u32{};

    const auto res1 = vkEnumeratePhysicalDevices(instance_ref, &count, nullptr);
    if (res1 != VK_SUCCESS) return std::unexpected{static_cast<result>(res1)};

    const auto res2 = vkEnumeratePhysicalDevices(instance_ref, &count, devices.data());
    if (res2 != VK_SUCCESS) return std::unexpected{static_cast<result>(res2)};

    return devices;
}

using physical_device_properties = VkPhysicalDeviceProperties;

[[nodiscard]] inline auto get_physical_device_properties(const physical_device& device) noexcept
    -> physical_device_properties {
    auto properties = physical_device_properties{};
    vkGetPhysicalDeviceProperties(device, &properties);
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

using queue_family_properties = VkQueueFamilyProperties;

[[nodiscard]] inline auto get_physical_device_queue_family_properties(
    const physical_device& device
) noexcept -> std::vector<queue_family_properties> {
    auto count = u32{};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    auto queue_families = std::vector<queue_family_properties>(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queue_families.data());

    return queue_families;
}

using device_queue_create_info = VkDeviceQueueCreateInfo;

class device_queue_create_info_setter final {
  public:
    explicit device_queue_create_info_setter(
        device_queue_create_info& info, const bool should_setup = true
    ) noexcept
        : info_{info} {
        if (not should_setup) return;
        info_       = {};
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
    explicit device_create_info_setter(device_create_info& info, const bool should_setup = true)
        : info_{info} {
        if (not should_setup) return;
        info_       = device_create_info{};
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

  private:
    device_create_info& info_;
};
}  // namespace hgn