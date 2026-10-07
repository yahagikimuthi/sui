#pragma once

#include <vulkan/vulkan.h>

#include "hgn/detail/others.hpp"
#include "hgn/types.hpp"

namespace hgn {
using application_info = VkApplicationInfo;

class application_info_setter final {
  public:
    explicit application_info_setter(
        application_info& info, const bool should_setup = true
    ) noexcept
        : info_{info} {
        if (not should_setup) return;
        info_       = {};
        info_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    }

    template <typename T>
        requires detail::is_string_literal_v<T>
    auto application_name(const T& name) noexcept -> application_info_setter& {
        info_.pApplicationName = name;
        return *this;
    }

    auto application_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info_setter& {
        info_.applicationVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    template <typename T>
        requires detail::is_string_literal_v<T>
    auto engine_name(const T& name) noexcept -> application_info_setter& {
        info_.pEngineName = name;
        return *this;
    }

    auto engine_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info_setter& {
        info_.engineVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    auto api_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info_setter& {
        info_.apiVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
        return *this;
    }

  private:
    application_info& info_;
};
}  // namespace hgn