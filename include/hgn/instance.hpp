#pragma once

#include <vulkan/vulkan.h>
#include <bit>
#include <cassert>
#include <expected>
#include <functional>
#include <string_view>
#include <vector>

#include "hgn/app_instance_info.hpp"
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

class layer_properties final {
  public:
    explicit layer_properties(const VkLayerProperties& native) : native_{native} {}

    [[nodiscard]] auto name() const noexcept -> std::string_view {
        const auto out = std::string_view{static_cast<const char*>(native_.layerName)};
        return out;
    }

  private:
    VkLayerProperties native_;
};

[[nodiscard]] inline auto try_enumerate_instance_layer_properties() noexcept
    -> std::expected<std::vector<layer_properties>, result> {
    auto count = u32{};
    if (auto res = vkEnumerateInstanceLayerProperties(&count, nullptr); res != VK_SUCCESS)
        return make_result(res);

    auto properties = std::vector<VkLayerProperties>(count);
    if (auto res = vkEnumerateInstanceLayerProperties(&count, properties.data()); res != VK_SUCCESS)
        return make_result(res);

    auto out = std::vector<layer_properties>{};
    out.reserve(count);

    for (auto& property : properties) {
        out.emplace_back(std::bit_cast<layer_properties>(property));
    }

    return out;
}

inline void destroy_instance(instance& ins) noexcept {
    vkDestroyInstance(&ins, nullptr);
}
}  // namespace hgn