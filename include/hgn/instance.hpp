#pragma once

#include <vulkan/vulkan.h>
#include <cassert>
#include <expected>
#include <string_view>

#include "hgn/app_instance_info.hpp"
#include "hgn/detail/experimenta.hpp"
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

class layer_properties final : public detail::wrapper<VkLayerProperties> {
  public:
    using wrapper<VkLayerProperties>::wrapper;

    [[nodiscard]] auto name() const noexcept -> std::string_view {
        const auto out = std::string_view{static_cast<const char*>(native().layerName)};
        return out;
    }
};

template <>
class view<layer_properties> final : public detail::base_view<layer_properties> {
  public:
    using base_view<layer_properties>::base_view;
    [[nodiscard]] auto name() const noexcept -> std::string_view {
        const auto out = std::string_view{static_cast<const char*>(native().layerName)};
        return out;
    }
};

[[nodiscard]] inline auto try_enumerate_instance_layer_properties() noexcept
    -> std::expected<proxy_vector<layer_properties>, result> {
    auto count = u32{};
    if (auto res = vkEnumerateInstanceLayerProperties(&count, nullptr); res != VK_SUCCESS)
        return make_result(res);

    auto properties = proxy_vector<layer_properties>(count);
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