#pragma once

#include <cstdint>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/hardware/instance/app_info.hpp"

namespace hgn::detail {
class instance_create_info_impl final {
  public:
    explicit instance_create_info_impl() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    }

    instance_create_info_impl(const instance_create_info_impl& other) noexcept
        : native_{other.native_},
          app_info_{other.app_info_},
          layer_{other.layer_},
          extensions_{other.extensions_} {
        reset();
    }

    auto operator=(const instance_create_info_impl& other) noexcept -> instance_create_info_impl& {
        if (this == &other) return *this;

        native_     = other.native_;
        app_info_   = other.app_info_;
        layer_      = other.layer_;
        extensions_ = other.extensions_;

        reset();
        return *this;
    }

    instance_create_info_impl(instance_create_info_impl&& other) noexcept
        : native_{other.native_},
          app_info_{std::move(other.app_info_)},
          layer_{other.layer_},
          extensions_{other.extensions_} {
        reset();
    }

    auto operator=(instance_create_info_impl&& other) noexcept -> instance_create_info_impl& {
        if (this == &other) return *this;

        native_     = other.native_;
        app_info_   = std::move(other.app_info_);
        layer_      = other.layer_;
        extensions_ = std::move(other.extensions_);

        reset();
        return *this;
    }

    ~instance_create_info_impl() noexcept = default;

    void app_info(const application_info& app) noexcept {
        app_info_                = app;
        native_.pApplicationInfo = &app_info_.get();
    }

    void layer(const std::string_view name) noexcept {
        layer_                      = name.data();
        native_.enabledLayerCount   = 1;
        native_.ppEnabledLayerNames = &layer_;
    }

    void extensions(const std::span<const std::string_view> names) noexcept {
        extensions_.reserve(names.size());
        extensions_.assign_range(
            names | std::views::transform([](const std::string_view view) noexcept -> const char* {
                return view.data();
            })
        );

        native_.enabledExtensionCount   = static_cast<std::uint32_t>(names.size());
        native_.ppEnabledExtensionNames = extensions_.data();
    }

    [[nodiscard]] auto get() const noexcept -> const VkInstanceCreateInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pApplicationInfo != nullptr) native_.pApplicationInfo = &app_info_.get();
        if (native_.ppEnabledLayerNames != nullptr) native_.ppEnabledLayerNames = &layer_;
        if (native_.ppEnabledExtensionNames != nullptr)
            native_.ppEnabledExtensionNames = extensions_.data();
    }

    VkInstanceCreateInfo     native_{};
    application_info         app_info_;
    const char*              layer_{};
    std::vector<const char*> extensions_;
};
}  // namespace hgn::detail