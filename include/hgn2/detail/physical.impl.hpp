#pragma once

#ifndef HGN_DETAIL_PHYSICAL_IMPL_HPP_
#include "hgn2/physical.hpp"
#endif

#include <cassert>
#include <cstdint>
#include <expected>
#include <ranges>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn2/enums.hpp"
#include "hgn2/instance.hpp"

namespace hgn {
inline auto instance::enumerate_physical() noexcept
    -> std::expected<std::vector<physical_device>, result> {
    auto count = std::uint32_t{};
    if (const auto res = vkEnumeratePhysicalDevices(&get(), &count, nullptr); res != VK_SUCCESS) {
        return make_unexpected(res);
    }

    auto devices = std::vector<VkPhysicalDevice_T*>(count);
    if (const auto res = vkEnumeratePhysicalDevices(&get(), &count, devices.data());
        res != VK_SUCCESS) {
        return make_unexpected(res);
    }

    return devices | std::views::transform([](VkPhysicalDevice_T* ptr) noexcept -> physical_device {
               assert(ptr != nullptr);
               return physical_device{*ptr};
           }) |
           std::ranges::to<std::vector>();
}
}  // namespace hgn