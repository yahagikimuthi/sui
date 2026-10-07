#pragma once

#include <vulkan/vulkan.h>
#include <span>

#include "hgn/application_info.hpp"

namespace hgn {
using instance_create_info = VkInstanceCreateInfo;

[[nodiscard]] inline auto make_instance_create_info(
    const application_info&      app,
    std::span<const char* const> layers,
    std::span<const char* const> extensions
) noexcept -> instance_create_info {
    auto info             = instance_create_info{};
    info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pNext            = nullptr;
    info.flags            = 0;
    info.pApplicationInfo = &app;

    info.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();

    info.enabledLayerCount   = static_cast<uint32_t>(layers.size());
    info.ppEnabledLayerNames = layers.data();

    return info;
}
}  // namespace hgn