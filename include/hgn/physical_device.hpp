#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <ranges>
#include <string_view>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/instance.hpp"

namespace hgn {
using physical_device = VkPhysicalDevice_T;

[[nodiscard]] inline auto try_enumerate_physical_devices(instance& instance_ref) noexcept
    -> std::expected<std::vector<std::reference_wrapper<physical_device>>, result> {
    auto count = std::uint32_t{};

    const auto res1 = vkEnumeratePhysicalDevices(&instance_ref, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto devices = std::vector<physical_device*>(count);

    const auto res2 = vkEnumeratePhysicalDevices(&instance_ref, &count, devices.data());
    if (res2 != VK_SUCCESS) return make_result(res2);

    auto out = std::vector<std::reference_wrapper<physical_device>>{};
    out.reserve(count);
    out.assign_range(
        devices |
        std::views::transform(
            [](physical_device* device) noexcept -> std::reference_wrapper<physical_device> {
                assert(device != nullptr);
                return std::ref(*device);
            }
        )
    );

    return out;
}

class physical_device_properties final {
  public:
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

    return std::bit_cast<physical_device_properties>(properties);
}

class queue_family_properties final {
  public:
    [[nodiscard]] auto flags() const noexcept -> queue_flag_bits {
        return static_cast<queue_flag_bits>(native_.queueFlags);
    }

  private:
    VkQueueFamilyProperties native_{};
};

[[nodiscard]] inline auto get_physical_device_queue_family_properties(
    physical_device& device
) noexcept -> std::vector<queue_family_properties> {
    auto count = std::uint32_t{};
    vkGetPhysicalDeviceQueueFamilyProperties(&device, &count, nullptr);

    auto queue_families = std::vector<VkQueueFamilyProperties>(count);
    vkGetPhysicalDeviceQueueFamilyProperties(&device, &count, queue_families.data());

    auto out = std::vector<queue_family_properties>{};
    out.reserve(count);
    out.assign_range(
        queue_families |
        std::views::transform(
            [](const VkQueueFamilyProperties& prop) noexcept -> queue_family_properties {
                return std::bit_cast<queue_family_properties>(prop);
            }
        )
    );

    return out;
}

class memory_type final {
  public:
    [[nodiscard]] auto property_flags() const noexcept -> memory_property_flags {
        return static_cast<memory_property_flags>(native_.propertyFlags);
    }

  private:
    VkMemoryType native_;
};

class physical_device_memory_properties final {
  public:
    [[nodiscard]] auto setting_memory_types(const std::size_t i) noexcept {
        const auto span = std::span<VkMemoryType>{native_.memoryTypes};
        return std::bit_cast<memory_type>(span[i]);
    }

  private:
    VkPhysicalDeviceMemoryProperties native_{};
};

[[nodiscard]] inline auto get_physical_device_memory_properties(physical_device& physical) noexcept
    -> physical_device_memory_properties {
    auto native = VkPhysicalDeviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(&physical, &native);
    return std::bit_cast<physical_device_memory_properties>(native);
}
}  // namespace hgn