#pragma once

#include <vulkan/vulkan.h>
#include <span>

#include "hgn/application_info.hpp"
#include "hgn/detail/wrapper.hpp"

namespace hgn {
using native_instance_create_info = VkInstanceCreateInfo;

class instance_create_info final : public detail::wrapper<VkInstanceCreateInfo> {
  public:
    explicit instance_create_info() { native().sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; }

    auto app_info(const application_info& app) noexcept -> instance_create_info& {
        native().pApplicationInfo = &app.native();
        return *this;
    }

    auto layers(const std::span<const char* const> vec) noexcept -> instance_create_info& {
        native().enabledLayerCount   = static_cast<u32>(vec.size());
        native().ppEnabledLayerNames = vec.data();
        return *this;
    }

    auto extensions(const std::span<const char* const> vec) noexcept -> instance_create_info& {
        native().enabledExtensionCount   = static_cast<u32>(vec.size());
        native().ppEnabledExtensionNames = vec.data();
        return *this;
    }
};
}  // namespace hgn