#pragma once

#include <vulkan/vulkan.h>
#include <span>

#include "hgn/application_info.hpp"

namespace hgn {
using instance_create_info = VkInstanceCreateInfo;

class instance_create_info_setter final {
  public:
    explicit instance_create_info_setter(instance_create_info& info) : info_{info} {
        info_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    }

    auto app_info(const application_info& app) noexcept -> instance_create_info_setter& {
        info_.pApplicationInfo = &app.native();
        return *this;
    }

    auto layers(const std::span<const char* const> vec) noexcept -> instance_create_info_setter& {
        info_.enabledLayerCount   = static_cast<u32>(vec.size());
        info_.ppEnabledLayerNames = vec.data();
        return *this;
    }

    auto extensions(const std::span<const char* const> vec) noexcept
        -> instance_create_info_setter& {
        info_.enabledExtensionCount   = static_cast<u32>(vec.size());
        info_.ppEnabledExtensionNames = vec.data();
        return *this;
    }

  private:
    instance_create_info& info_;
};
}  // namespace hgn