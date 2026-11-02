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
        if (not is_debug) return *this;

        layer_ = "VK_LAYER_KHRONOS_validation";
        extensions_.emplace_back("VK_EXT_debug_utils");
        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::expected<instance, result> {
        app_.pApplicationName = app_name_.c_str();
        app_.pEngineName      = engine_name_.c_str();

        info_.pApplicationInfo    = &app_;
        info_.enabledLayerCount   = 1;
        info_.ppEnabledLayerNames = &layer_;

        info_.enabledExtensionCount   = static_cast<std::uint32_t>(extensions_.size());
        info_.ppEnabledExtensionNames = extensions_.data();

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

        auto   count      = std::uint32_t{};
        auto** extensions = glfwGetRequiredInstanceExtensions(&count);
        extensions_.reserve(count);
        extensions_.assign_range(std::span{extensions, count});
    }

    VkApplicationInfo        app_{};
    VkInstanceCreateInfo     info_{};
    std::string              app_name_;
    std::string              engine_name_;
    const char*              layer_{};
    std::vector<const char*> extensions_;
};
}  // namespace hgn
