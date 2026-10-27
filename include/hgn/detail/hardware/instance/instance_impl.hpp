#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <ranges>
#include <string_view>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/hardware/instance/info.hpp"

namespace hgn::detail {
using instance_impl = VkInstance_T;

[[nodiscard]] inline auto try_make_instance_impl(const instance_create_info& info) noexcept
    -> std::expected<std::reference_wrapper<instance_impl>, result> {
    auto*      ptr = static_cast<instance_impl*>(nullptr);
    const auto res = vkCreateInstance(&info.get(), nullptr, &ptr);
    if (res != VK_SUCCESS) return make_result(res);

    assert(ptr != nullptr);
    return std::ref(*ptr);
}

class layer_properties_impl final {
  public:
    explicit layer_properties_impl() noexcept = delete;

    [[nodiscard]] auto name() const noexcept -> std::string_view {
        const auto out = std::string_view{static_cast<const char*>(native_.layerName)};
        return out;
    }

  private:
    VkLayerProperties native_;
};

[[nodiscard]] inline auto try_enumerate_instance_layer_properties_impl() noexcept
    -> std::expected<std::vector<layer_properties_impl>, result> {
    auto count = std::uint32_t{};
    if (auto res = vkEnumerateInstanceLayerProperties(&count, nullptr); res != VK_SUCCESS)
        return make_result(res);

    auto properties = std::vector<VkLayerProperties>(count);
    if (auto res = vkEnumerateInstanceLayerProperties(&count, properties.data()); res != VK_SUCCESS)
        return make_result(res);

    auto out = std::vector<layer_properties_impl>{};
    out.reserve(count);
    out.assign_range(
        properties |
        std::views::transform([](const VkLayerProperties& prop) noexcept -> layer_properties_impl {
            return std::bit_cast<layer_properties_impl>(prop);
        })
    );

    return out;
}

inline void destroy_instance_impl(instance_impl& target) noexcept {
    vkDestroyInstance(&target, nullptr);
}
}  // namespace hgn::detail