#pragma once

#include <vulkan/vulkan.h>
#include <span>

#include "hgn/application_info.hpp"

namespace hgn {
using native_instance_create_info = VkInstanceCreateInfo;

class instance_create_info final {
  public:
    explicit instance_create_info() { native_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; }

    auto app_info(const application_info& app) noexcept -> instance_create_info& {
        native_.pApplicationInfo = &app.native();
        return *this;
    }

    auto layers(const std::span<const char* const> vec) noexcept -> instance_create_info& {
        native_.enabledLayerCount   = static_cast<u32>(vec.size());
        native_.ppEnabledLayerNames = vec.data();
        return *this;
    }

    auto extensions(const std::span<const char* const> vec) noexcept -> instance_create_info& {
        native_.enabledExtensionCount   = static_cast<u32>(vec.size());
        native_.ppEnabledExtensionNames = vec.data();
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const native_instance_create_info& {
        return native_;
    }

  private:
    native_instance_create_info native_{};
};
}  // namespace hgn