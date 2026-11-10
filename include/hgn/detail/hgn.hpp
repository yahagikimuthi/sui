#pragma once

#include <cstdint>
#include <expected>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/instance.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class instance_creator final {
  public:
    explicit instance_creator() noexcept {
        app_.sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_.apiVersion = VK_API_VERSION_1_3;

        info_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    }

    auto app_name(const std::string_view name) noexcept -> instance_creator& {
        app_name_ = name;
        return *this;
    }

    auto app_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept -> instance_creator& {
        app_.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
        return *this;
    }

    auto engine_name(const std::string_view name) noexcept -> instance_creator& {
        engine_name_ = name;
        return *this;
    }

    auto engine_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept -> instance_creator& {
        app_.engineVersion = VK_MAKE_VERSION(major, minor, patch);
        return *this;
    }

    auto extensions(const std::span<const std::string_view> extensions
    ) noexcept -> instance_creator& {
        extensions_.assign_range(
            extensions | std::views::transform([](std::string_view view) noexcept -> const char* {
                return view.data();
            })
        );
        return *this;
    }

    auto debug(const bool is_debug) noexcept -> instance_creator& {
        debug_ = is_debug;
        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::expected<instance, result> {
        app_.pApplicationName = app_name_.c_str();
        app_.pEngineName      = engine_name_.c_str();

        info_.pApplicationInfo = &app_;

        // レイヤーのセット
        constexpr auto layer = "VK_LAYER_KHRONOS_validation";
        if (debug_) {
            info_.enabledLayerCount   = 1;
            info_.ppEnabledLayerNames = &layer;
        }

        // 拡張機能のセット
        if (debug_) extensions_.emplace_back("VK_EXT_debug_utils");

        info_.enabledExtensionCount   = static_cast<std::uint32_t>(extensions_.size());
        info_.ppEnabledExtensionNames = extensions_.data();

        auto* raw = static_cast<raw::instance*>(nullptr);
        return make_expected(vkCreateInstance(&info_, nullptr, &raw))
            .transform([&]() noexcept -> instance { return instance{*raw}; });
    }

  private:
    VkApplicationInfo        app_{};
    VkInstanceCreateInfo     info_{};
    std::string              app_name_;
    std::string              engine_name_;
    std::vector<const char*> extensions_;
    bool                     debug_{false};
};
}  // namespace hgn
