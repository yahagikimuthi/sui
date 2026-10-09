#pragma once

#include <vulkan/vulkan.h>
#include <cassert>
#include <expected>
#include <optional>
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

class layer_properties final {};

class layer_properties_view final {
  public:
    explicit layer_properties_view(native_layer_properties& prop) noexcept : prop_{prop} {}

    [[nodiscard]] auto name() const noexcept -> std::string_view {
        const auto out = std::string_view{static_cast<char*>(prop_->layerName)};
        return out;
    }

  private:
    std::optional<native_layer_properties&> prop_;
};

template <>
class vector<layer_properties>
    : public detail::base_vector<native_layer_properties, layer_properties_view> {
    using detail::base_vector<native_layer_properties, layer_properties_view>::base_vector;
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