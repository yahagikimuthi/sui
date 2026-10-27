#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/detail/others.hpp"

namespace hgn::detail {
class application_info_impl final {
  public:
    explicit application_info_impl() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    }

    application_info_impl(const application_info_impl& other) noexcept
        : native_{other.native_}, app_name_{other.app_name_}, engine_name_{other.engine_name_} {
        reset();
    }

    auto operator=(const application_info_impl& other) noexcept -> application_info_impl& {
        if (this == &other) return *this;

        native_      = other.native_;
        app_name_    = other.app_name_;
        engine_name_ = other.engine_name_;

        reset();
        return *this;
    }

    application_info_impl(application_info_impl&& other) noexcept
        : native_{other.native_},
          app_name_{std::move(other.app_name_)},
          engine_name_{std::move(other.engine_name_)} {
        reset();
    }

    auto operator=(application_info_impl&& other) noexcept -> application_info_impl& {
        if (this == &other) return *this;

        native_      = other.native_;
        app_name_    = std::move(other.app_name_);
        engine_name_ = std::move(other.engine_name_);

        reset();
        return *this;
    }

    ~application_info_impl() noexcept = default;

    void app_name(const std::string_view name) noexcept {
        app_name_                = name;
        native_.pApplicationName = app_name_.c_str();
    }

    void app_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept {
        native_.applicationVersion = make_version(major, minor, patch);
    }

    void engine_name(const std::string_view name) noexcept {
        engine_name_        = name;
        native_.pEngineName = engine_name_.c_str();
    }

    void engine_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept {
        native_.engineVersion = make_version(major, minor, patch);
    }

    void api_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept {
        native_.apiVersion = make_version(major, minor, patch);
    }

    [[nodiscard]] auto get() const noexcept -> const VkApplicationInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pApplicationName != nullptr) native_.pApplicationName = app_name_.c_str();
        if (native_.pEngineName != nullptr) native_.pEngineName = engine_name_.c_str();
    }

    VkApplicationInfo native_{};
    std::string       app_name_;
    std::string       engine_name_;
};
}  // namespace hgn::detail