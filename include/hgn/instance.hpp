#pragma once

#include <vulkan/vulkan.h>
#include <expected>
#include <vector>

#include "hgn/create_info.hpp"
#include "hgn/result.hpp"

namespace hgn {
using instance = VkInstance;

[[nodiscard]] inline auto try_make_instance(const instance_create_info& info) noexcept
    -> std::expected<instance, result> {
    auto*      ins = instance{null_handle};
    const auto res = vkCreateInstance(&info, nullptr, &ins);
    if (res != VK_SUCCESS) return std::unexpected{static_cast<result>(res)};
    return ins;
}

using layer_properties = VkLayerProperties;

[[nodiscard]] inline auto enumerate_instance_layer_properties() noexcept
    -> std::expected<std::vector<layer_properties>, result> {
    auto count = u32{};
    if (auto res = vkEnumerateInstanceLayerProperties(&count, nullptr); res != VK_SUCCESS)
        return std::unexpected{static_cast<result>(res)};

    auto properties = std::vector<layer_properties>(count);
    if (count <= 0) return properties;
    if (auto res = vkEnumerateInstanceLayerProperties(&count, properties.data()); res != VK_SUCCESS)
        return std::unexpected{static_cast<result>(res)};

    return properties;
}
}  // namespace hgn