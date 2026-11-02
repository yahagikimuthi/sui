#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/instance.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class instance_creator final {
  public:
    friend auto get_instance_creator() noexcept -> instance_creator;

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
        auto   count        = std::uint32_t{};
        auto** p_extensions = glfwGetRequiredInstanceExtensions(&count);
        auto   extensions   = std::vector<const char*>{};
        extensions.assign_range(std::span{p_extensions, count});
        if (debug_) extensions.emplace_back("VK_EXT_debug_utils");

        info_.enabledExtensionCount   = static_cast<std::uint32_t>(extensions.size());
        info_.ppEnabledExtensionNames = extensions.data();

        auto*      raw = static_cast<raw::instance*>(nullptr);
        const auto res = vkCreateInstance(&info_, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return instance{*raw};
    }

  private:
    explicit instance_creator() noexcept {
        app_.sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_.apiVersion = VK_API_VERSION_1_3;

        info_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    }

    VkApplicationInfo    app_{};
    VkInstanceCreateInfo info_{};
    std::string          app_name_;
    std::string          engine_name_;
    bool                 debug_{false};
};
}  // namespace hgn
