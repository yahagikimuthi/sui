#pragma once

#include <vulkan/vulkan.h>
#include <expected>
#include <vector>

#include "hgn/instance.hpp"
#include "hgn/result.hpp"
#include "hgn/types.hpp"

namespace hgn {
using physical_device = VkPhysicalDevice;

[[nodiscard]] inline auto enumerate_physical_devices(const instance& instance_ref) noexcept
    -> std::expected<std::vector<physical_device>, result> {
    auto devices = std::vector<physical_device>{};
    auto count   = u32{};

    const auto res1 = vkEnumeratePhysicalDevices(instance_ref, &count, nullptr);
    if (res1 != VK_SUCCESS) return std::unexpected{static_cast<result>(res1)};

    const auto res2 = vkEnumeratePhysicalDevices(instance_ref, &count, devices.data());
    if (res2 != VK_SUCCESS) return std::unexpected{static_cast<result>(res2)};

    return devices;
}
}  // namespace hgn