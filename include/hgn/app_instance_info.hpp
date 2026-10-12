#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "hgn/detail/others.hpp"

namespace hgn {
class application_info final {
  public:
    explicit application_info() noexcept { native_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; }

    application_info(const application_info& other) noexcept
        : native_{other.native_}, app_name_{other.app_name_}, engine_name_{other.engine_name_} {
        reset();
    }

    auto operator=(const application_info& other) noexcept -> application_info& {
        if (this == &other) return *this;

        native_      = other.native_;
        app_name_    = other.app_name_;
        engine_name_ = other.engine_name_;

        reset();
        return *this;
    }

    application_info(application_info&& other) noexcept
        : native_{other.native_},
          app_name_{std::move(other.app_name_)},
          engine_name_{std::move(other.engine_name_)} {
        reset();
    }

    auto operator=(application_info&& other) noexcept -> application_info& {
        if (this == &other) return *this;

        native_      = other.native_;
        app_name_    = std::move(other.app_name_);
        engine_name_ = std::move(other.engine_name_);

        reset();
        return *this;
    }

    ~application_info() noexcept = default;

    auto application_name(const std::string_view name) noexcept -> application_info& {
        app_name_                = name;
        native_.pApplicationName = app_name_.c_str();
        return *this;
    }

    auto application_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept -> application_info& {
        native_.applicationVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    auto engine_name(const std::string_view name) noexcept -> application_info& {
        engine_name_        = name;
        native_.pEngineName = engine_name_.c_str();
        return *this;
    }

    auto engine_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept -> application_info& {
        native_.engineVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    auto api_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept -> application_info& {
        native_.apiVersion = detail::make_api_version(0, major, minor, patch);
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkApplicationInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pApplicationName != nullptr) native_.pApplicationName = app_name_.c_str();
        if (native_.pEngineName != nullptr) native_.pEngineName = engine_name_.c_str();
    }

    VkApplicationInfo native_{};
    std::string       app_name_;
    std::string       engine_name_;
};

class instance_create_info final {
  public:
    explicit instance_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    }

    instance_create_info(const instance_create_info& other) noexcept
        : native_{other.native_},
          app_info_{other.app_info_},
          layers_{other.layers_},
          extensions_{other.extensions_} {
        reset();
    }

    auto operator=(const instance_create_info& other) noexcept -> instance_create_info& {
        if (this == &other) return *this;

        native_     = other.native_;
        app_info_   = other.app_info_;
        layers_     = other.layers_;
        extensions_ = other.extensions_;

        reset();
        return *this;
    }

    instance_create_info(instance_create_info&& other) noexcept
        : native_{other.native_},
          app_info_{other.app_info_},
          layers_{std::move(other.layers_)},
          extensions_{std::move(other.extensions_)} {
        reset();
    }

    auto operator=(instance_create_info&& other) noexcept -> instance_create_info& {
        if (this == &other) return *this;

        native_     = other.native_;
        app_info_   = other.app_info_;
        layers_     = std::move(other.layers_);
        extensions_ = std::move(other.extensions_);

        reset();
        return *this;
    }

    ~instance_create_info() noexcept = default;

    auto app_info(const application_info& app) noexcept -> instance_create_info& {
        app_info_                = app.native();
        native_.pApplicationInfo = &app_info_;
        return *this;
    }

    auto layers(const std::span<const char* const> vec) noexcept -> instance_create_info& {
        layers_.reserve(vec.size());
        layers_.assign_range(vec);
        native_.enabledLayerCount   = static_cast<std::uint32_t>(layers_.size());
        native_.ppEnabledLayerNames = layers_.data();
        return *this;
    }

    auto extensions(const std::span<const char* const> vec) noexcept -> instance_create_info& {
        extensions_.reserve(vec.size());
        extensions_.assign_range(vec);
        native_.enabledExtensionCount   = static_cast<std::uint32_t>(extensions_.size());
        native_.ppEnabledExtensionNames = extensions_.data();
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkInstanceCreateInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pApplicationInfo != nullptr) native_.pApplicationInfo = &app_info_;
        if (native_.ppEnabledLayerNames != nullptr) native_.ppEnabledLayerNames = layers_.data();
        if (native_.ppEnabledExtensionNames != nullptr)
            native_.ppEnabledExtensionNames = extensions_.data();
    }

    VkInstanceCreateInfo     native_{};
    VkApplicationInfo        app_info_{};
    std::vector<const char*> layers_;
    std::vector<const char*> extensions_;
};
}  // namespace hgn