#pragma once

#include <vulkan/vulkan.h>
#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <ranges>
#include <string_view>
#include <vector>

#include "hgn/app_instance_info.hpp"
#include "hgn/enums.hpp"

namespace hgn {
using instance = VkInstance_T;

[[nodiscard]] inline auto try_make_instance(const instance_create_info& info) noexcept
    -> std::expected<std::reference_wrapper<instance>, result> {
    auto*      ins = static_cast<instance*>(nullptr);
    const auto res = vkCreateInstance(&info.native(), nullptr, &ins);
    if (res != VK_SUCCESS) return make_result(res);

    assert(ins != nullptr);
    return std::ref(*ins);
}

class layer_properties final {
  public:
    [[nodiscard]] auto name() const noexcept -> std::string_view {
        const auto out = std::string_view{static_cast<const char*>(native_.layerName)};
        return out;
    }

  private:
    VkLayerProperties native_;
};

[[nodiscard]] inline auto try_enumerate_instance_layer_properties() noexcept
    -> std::expected<std::vector<layer_properties>, result> {
    auto count = std::uint32_t{};
    if (auto res = vkEnumerateInstanceLayerProperties(&count, nullptr); res != VK_SUCCESS)
        return make_result(res);

    auto properties = std::vector<VkLayerProperties>(count);
    if (auto res = vkEnumerateInstanceLayerProperties(&count, properties.data()); res != VK_SUCCESS)
        return make_result(res);

    auto out = std::vector<layer_properties>{};
    out.reserve(count);
    out.assign_range(
        properties |
        std::views::transform([](const VkLayerProperties& prop) noexcept -> layer_properties {
            return std::bit_cast<layer_properties>(prop);
        })
    );

    return out;
}

inline void destroy_instance(instance& ins) noexcept {
    vkDestroyInstance(&ins, nullptr);
}
}  // namespace hgn