#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <optional>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/app_instance_info.hpp"
#include "hgn/enums.hpp"

namespace hgn {
class instance final {
  public:
    [[nodiscard]] static auto try_make(const instance_create_info& info) noexcept
        -> std::expected<instance, result> {
        auto*      native = static_cast<VkInstance_T*>(nullptr);
        const auto res    = vkCreateInstance(&info.get(), nullptr, &native);
        if (res != VK_SUCCESS) return make_result(res);

        assert(native != nullptr);
        return instance{*native};
    }

    instance(const instance&) noexcept                    = delete;
    auto operator=(const instance&) noexcept -> instance& = delete;

    instance(instance&& other) noexcept : native_{std::exchange(other.native_, std::nullopt)} {}

    auto operator=(instance&& other) noexcept -> instance& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        return *this;
    }

    ~instance() { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkInstance_T& { return *native_; }

  private:
    explicit instance(VkInstance_T& native) noexcept : native_{native} {}

    void destroy() noexcept {
        if (native_) vkDestroyInstance(&get(), nullptr);
    }

    std::optional<VkInstance_T&> native_;
};

[[nodiscard]] inline auto try_make_instance(const instance_create_info& info) noexcept
    -> std::expected<instance, result> {
    return instance::try_make(info);
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
}  // namespace hgn