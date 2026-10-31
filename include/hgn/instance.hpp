#pragma once

#include <cassert>
#include <expected>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "hgn/application.hpp"
#include "hgn/enums.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw.hpp"

namespace hgn {
class physical_device;
class instance final {
  public:
    instance(const instance&) noexcept                    = delete;
    auto operator=(const instance&) noexcept -> instance& = delete;
    instance(instance&&) noexcept                         = default;
    auto operator=(instance&&) noexcept -> instance&      = default;
    ~instance() noexcept                                  = default;

    [[nodiscard]] auto enumerate_physical() noexcept
        -> std::expected<std::vector<physical_device>, result>;

    [[nodiscard]] auto get() noexcept -> raw::instance& { return raw_; }

    class builder final {
      public:
        explicit builder() noexcept {
            auto   count      = std::uint32_t{};
            auto** extensions = glfwGetRequiredInstanceExtensions(&count);

            auto span = std::span<const char*>(extensions, count);
            extensions_.reserve(span.size());
            extensions_.assign_range(span);
        }

        auto app(const application& app) noexcept -> builder& {
            app_ = app;
            return *this;
        }

        auto debug(const bool is_debug) noexcept -> builder& {
            if (not is_debug) return *this;

            layer_ = "VK_LAYER_KHRONOS_validation";
            extensions_.emplace_back("VK_EXT_debug_utils");
            return *this;
        }

        [[nodiscard]] auto build() const noexcept -> std::expected<instance, result> {
            auto info  = VkInstanceCreateInfo{};
            info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            if (app_) {
                info.pApplicationInfo = &app_->info();
            }
            info.enabledLayerCount   = 1;
            info.ppEnabledLayerNames = &layer_;

            info.enabledExtensionCount   = static_cast<std::uint32_t>(extensions_.size());
            info.ppEnabledExtensionNames = extensions_.data();

            auto* raw = static_cast<raw::instance*>(nullptr);

            const auto res = vkCreateInstance(&info, nullptr, &raw);
            if (res != VK_SUCCESS) return make_unexpected(res);

            assert(raw != nullptr);
            return instance{*raw};
        }

      private:
        std::optional<application> app_;
        const char*                layer_{};
        std::vector<const char*>   extensions_;
    };

  private:
    friend class instance::builder;

    explicit instance(raw::instance& raw) noexcept : raw_{std::move(raw)} {}

    raii::instance raw_;
};
}  // namespace hgn