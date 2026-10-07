#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <array>
#include <span>
#include <vector>

#include "hgn/types.hpp"

namespace hgn {
inline constexpr auto null_handle = VK_NULL_HANDLE;

using layer_properties = VkLayerProperties;

[[nodiscard]] inline auto enumerate_instance_layer_properties() noexcept
    -> std::vector<layer_properties> {
    auto count = u32{};
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    auto properties = std::vector<layer_properties>(count);
    if (count <= 0) return properties;
    vkEnumerateInstanceLayerProperties(&count, properties.data());

    return properties;
}

[[nodiscard]] inline auto get_required_instance_extensions() -> std::vector<const char*> {
    auto         count          = u32{};
    const char** extensions_ptr = glfwGetRequiredInstanceExtensions(&count);

    if (extensions_ptr == nullptr or count == 0) {
        return {};
    }

    // ポインタ範囲指定コンストラクタで vector にまとめて返す
    return {extensions_ptr, extensions_ptr + count};  // NOLINT
}
}  // namespace hgn