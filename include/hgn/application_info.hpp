#pragma once

#include <vulkan/vulkan.h>

#include "hgn/detail/others.hpp"
#include "hgn/detail/wrapper.hpp"
#include "hgn/types.hpp"

namespace hgn {
class application_info final : public detail::wrapper<VkApplicationInfo> {
  public:
    explicit application_info() noexcept { native().sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; }

    template <typename T>
        requires detail::is_string_literal_v<T>
    auto application_name(const T& name) noexcept -> application_info& {
        native().pApplicationName = name;
        return *this;
    }

    auto application_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        native().applicationVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    template <typename T>
        requires detail::is_string_literal_v<T>
    auto engine_name(const T& name) noexcept -> application_info& {
        native().pEngineName = name;
        return *this;
    }

    auto engine_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        native().engineVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    auto api_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        native().apiVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
        return *this;
    }
};
}  // namespace hgn