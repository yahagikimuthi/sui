#pragma once

#include <vulkan/vulkan.h>
#include <expected>
#include <optional>
#include <vector>

#include "hgn/create_info.hpp"
#include "hgn/result.hpp"

namespace hgn {
using instance = VkInstance;

[[nodiscard]] inline auto try_make_instance(instance_create_info& info) noexcept
    -> std::optional<instance> {
    auto*      ins    = instance{null_handle};
    const auto result = vkCreateInstance(&info, nullptr, &ins);
    if (result != VK_SUCCESS) return std::nullopt;
    return ins;
}

using layer_properties = VkLayerProperties;

[[nodiscard]] inline auto enumerate_instance_layer_properties() noexcept
    -> std::expected<std::vector<layer_properties>, result> {
    auto       count = u32{};
    const auto res1  = vkEnumerateInstanceLayerProperties(&count, nullptr);
    if (res1 != VK_SUCCESS) return std::unexpected{static_cast<result>(res1)};

    auto properties = std::vector<layer_properties>(count);
    if (count <= 0) return properties;
    const auto res2 = vkEnumerateInstanceLayerProperties(&count, properties.data());

    if (res2 != VK_SUCCESS) return std::unexpected{static_cast<result>(res1)};

    return properties;
}
}  // namespace hgn