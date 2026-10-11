#pragma once

#include <vulkan/vulkan.h>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "hgn/detail/others.hpp"
#include "hgn/types.hpp"

namespace hgn {
class application_info final {
  public:
    explicit application_info() noexcept { native_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; }

    auto application_name(const std::string_view name) noexcept -> application_info& {
        app_name_                = name;
        native_.pApplicationName = app_name_.c_str();
        return *this;
    }

    auto application_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        native_.applicationVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    auto engine_name(const std::string_view name) noexcept -> application_info& {
        engine_name_        = name;
        native_.pEngineName = engine_name_.c_str();
        return *this;
    }

    auto engine_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        native_.engineVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    auto api_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        native_.apiVersion = detail::make_api_version(0, major, minor, patch);
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkApplicationInfo& { return native_; }

  private:
    VkApplicationInfo native_{};
    std::string       app_name_;
    std::string       engine_name_;
};

class instance_create_info final {
  public:
    explicit instance_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    }

    auto app_info(const application_info& app) noexcept -> instance_create_info& {
        app_info_                = app.native();
        native_.pApplicationInfo = &app_info_;
        return *this;
    }

    auto layers(const std::span<const char* const> vec) noexcept -> instance_create_info& {
        layers_.assign_range(vec);
        native_.enabledLayerCount   = static_cast<u32>(layers_.size());
        native_.ppEnabledLayerNames = layers_.data();
        return *this;
    }

    auto extensions(const std::span<const char* const> vec) noexcept -> instance_create_info& {
        extensions_.assign_range(vec);
        native_.enabledExtensionCount   = static_cast<u32>(extensions_.size());
        native_.ppEnabledExtensionNames = extensions_.data();
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkInstanceCreateInfo& { return native_; }

  private:
    VkInstanceCreateInfo     native_{};
    VkApplicationInfo        app_info_{};
    std::vector<const char*> layers_;
    std::vector<const char*> extensions_;
};
}  // namespace hgn