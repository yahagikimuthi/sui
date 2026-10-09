#pragma once

#include <vulkan/vulkan.h>
#include <cassert>
#include <expected>
#include <string_view>

#include "hgn/create_info.hpp"
#include "hgn/detail/vector.hpp"
#include "hgn/result.hpp"
#include "hgn/types.hpp"

namespace hgn {
using instance = VkInstance_T;

[[nodiscard]] inline auto try_make_instance(const instance_create_info& info) noexcept
    -> std::expected<ref_w<instance>, result> {
    auto*      ins = static_cast<instance*>(nullptr);
    const auto res = vkCreateInstance(&info.native(), nullptr, &ins);
    if (res != VK_SUCCESS) return make_result(res);

    assert(ins != nullptr);
    return std::ref(*ins);
}

using native_layer_properties = VkLayerProperties;

class layer_properties final : public detail::wrapper<native_layer_properties> {
  public:
    using wrapper<native_layer_properties>::wrapper;

    [[nodiscard]] auto name() const noexcept -> std::string_view {
        const auto out = std::string_view{static_cast<const char*>(native().layerName)};
        return out;
    }
};

template <>
class vector<layer_properties>
    : public detail::base_vector<native_layer_properties, layer_properties> {
    using detail::base_vector<native_layer_properties, layer_properties>::base_vector;
};

[[nodiscard]] inline auto try_enumerate_instance_layer_properties() noexcept
    -> std::expected<vector<layer_properties>, result> {
    auto count = u32{};
    if (auto res = vkEnumerateInstanceLayerProperties(&count, nullptr); res != VK_SUCCESS)
        return make_result(res);

    auto properties = vector<layer_properties>(count);
    if (count <= 0) return properties;
    if (auto res = vkEnumerateInstanceLayerProperties(&count, properties.native_data());
        res != VK_SUCCESS)
        return make_result(res);

    return properties;
}

inline void destroy_instance(instance& ins) noexcept {
    vkDestroyInstance(&ins, nullptr);
}
}  // namespace hgn