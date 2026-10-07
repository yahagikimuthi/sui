#pragma once

#include <vulkan/vulkan.h>
#include <optional>

#include "hgn/create_info.hpp"
#include "hgn/others.hpp"

namespace hgn {
using instance = VkInstance;

[[nodiscard]] inline auto try_make_instance(instance_create_info& info) noexcept
    -> std::optional<instance> {
    auto*      ins    = instance{null_handle};
    const auto result = vkCreateInstance(&info, nullptr, &ins);
    if (result != VK_SUCCESS) return std::nullopt;
    return ins;
}
}  // namespace hgn