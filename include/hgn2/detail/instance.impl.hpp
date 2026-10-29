#pragma once

#ifndef HGN_DETAIL_INSTANCE_IMPL_HPP_
#include "hgn2/instance.hpp"
#endif

#include <cassert>
#include <cstdint>
#include <expected>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn2/application.hpp"
#include "hgn2/enums.hpp"

namespace hgn {

inline instance::instance(instance&& other) noexcept
    : raw_{std::exchange(other.raw_, std::nullopt)} {}

inline auto instance::operator=(instance&& other) noexcept -> instance& {
    if (this == &other) return *this;

    destroy();

    raw_ = std::exchange(other.raw_, std::nullopt);
    return *this;
}

inline auto instance::builder::app(const application& app) noexcept -> builder& {
    app_ = app;
    return *this;
}

inline auto instance::builder::layer(const std::string_view layer) noexcept -> builder& {
    layer_ = layer;
    return *this;
}

inline auto instance::builder::extensions(const std::span<const std::string_view> exts) noexcept
    -> builder& {
    extensions_.reserve(exts.size());
    extensions_.assign_range(
        exts | std::views::transform([](const std::string_view view) noexcept -> std::string {
            return std::string{view};
        })
    );
    return *this;
}

inline auto instance::builder::build() noexcept -> std::expected<instance, result> {
    auto info  = VkInstanceCreateInfo{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    if (app_) {
        info.pApplicationInfo = &app_->info();
    }
    info.enabledLayerCount      = 1;
    const char* const layer_ptr = layer_.c_str();
    info.ppEnabledLayerNames    = &layer_ptr;

    info.enabledExtensionCount = static_cast<std::uint32_t>(extensions_.size());
    const auto extensions =
        extensions_ | std::views::transform([](const std::string_view str) noexcept -> const char* {
            return str.data();
        }) |
        std::ranges::to<std::vector>();
    info.ppEnabledExtensionNames = extensions.data();

    auto* raw = static_cast<VkInstance_T*>(nullptr);

    const auto res = vkCreateInstance(&info, nullptr, &raw);
    if (res != VK_SUCCESS) return make_unexpected(res);

    assert(raw != nullptr);
    return instance{*raw};
}

inline void instance::destroy() noexcept {
    if (raw_) {
        vkDestroyInstance(&get(), nullptr);
    }
}
}  // namespace hgn
