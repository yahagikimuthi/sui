#pragma once

#include <vulkan/vulkan.h>
#include <cassert>
#include <expected>
#include <vector>

#include "hgn/create_info.hpp"
#include "hgn/result.hpp"
#include "hgn/types.hpp"

namespace hgn {
using instance = VkInstance_T;

[[nodiscard]] inline auto try_make_instance(const instance_create_info& info) noexcept
    -> std::expected<ref_w<instance>, result> {
    auto*      ins = static_cast<instance*>(nullptr);
    const auto res = vkCreateInstance(&info, nullptr, &ins);
    if (res != VK_SUCCESS) return detail::make_result(res);

    assert(ins != nullptr);
    return std::ref(*ins);
}

using layer_properties = VkLayerProperties;

[[nodiscard]] inline auto try_enumerate_instance_layer_properties() noexcept
    -> std::expected<std::vector<layer_properties>, result> {
    auto count = u32{};
    if (auto res = vkEnumerateInstanceLayerProperties(&count, nullptr); res != VK_SUCCESS)
        return detail::make_result(res);

    auto properties = std::vector<layer_properties>(count);
    if (count <= 0) return properties;
    if (auto res = vkEnumerateInstanceLayerProperties(&count, properties.data()); res != VK_SUCCESS)
        return detail::make_result(res);

    return properties;
}

inline void destroy_instance(instance& ins) noexcept {
    vkDestroyInstance(&ins, nullptr);
}
}  // namespace hgn