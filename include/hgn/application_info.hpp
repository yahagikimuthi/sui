#pragma once

#include <vulkan/vulkan.h>

#include "hgn/detail/others.hpp"
#include "hgn/types.hpp"

namespace hgn {
using application_info = VkApplicationInfo;

[[nodiscard]] constexpr auto make_version(
    const u32 major, const u32 minor, const u32 patch
) noexcept -> u32 {
    return VK_MAKE_VERSION(major, minor, patch);
}

inline constexpr auto api_version_1_3 = VK_API_VERSION_1_3;

template <typename T, typename U>
    requires detail::is_string_literal_v<T> and detail::is_string_literal_v<U>
[[nodiscard]] inline auto make_application_info(
    const T&  app_name,
    const u32 app_version,
    const U&  engine_name,
    const u32 engine_version,
    const u32 api_version
) noexcept -> application_info {
    auto info               = VkApplicationInfo{};
    info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pNext              = nullptr;
    info.pApplicationName   = app_name;
    info.applicationVersion = app_version;
    info.pEngineName        = engine_name;
    info.engineVersion      = engine_version;
    info.apiVersion         = api_version;
    return info;
}

}  // namespace hgn