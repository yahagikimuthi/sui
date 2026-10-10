#pragma once

#include <vulkan/vulkan.h>
#include <span>

#include "hgn/detail/experimenta.hpp"
#include "hgn/detail/others.hpp"
#include "hgn/types.hpp"

namespace hgn {
class application_info final : public detail::wrapper<VkApplicationInfo> {
  public:
    explicit application_info() noexcept : wrapper(VkApplicationInfo{}) {
        native().sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    }

    template <typename T>
        requires detail::is_string_literal_v<T>
    auto application_name(const T& name) noexcept -> application_info& {
        native().pApplicationName = name;
        return *this;
    }

    auto application_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        native().applicationVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    template <typename T>
        requires detail::is_string_literal_v<T>
    auto engine_name(const T& name) noexcept -> application_info& {
        native().pEngineName = name;
        return *this;
    }

    auto engine_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        native().engineVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    auto api_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        native().apiVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
        return *this;
    }
};

template <>
class view<application_info> final : public detail::base_view<application_info> {
    using base_view<application_info>::base_view;

  public:
    template <typename T>
        requires detail::is_string_literal_v<T>
    auto application_name(const T& name) noexcept -> view<application_info>& {
        native().pApplicationName = name;
        return *this;
    }

    auto application_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> view<application_info>& {
        native().applicationVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    template <typename T>
        requires detail::is_string_literal_v<T>
    auto engine_name(const T& name) noexcept -> view<application_info>& {
        native().pEngineName = name;
        return *this;
    }

    auto engine_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> view<application_info>& {
        native().engineVersion = detail::make_version(major, minor, patch);
        return *this;
    }

    auto api_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> view<application_info>& {
        native().apiVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
        return *this;
    }
};

class instance_create_info final {
  public:
    explicit instance_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    }

    auto app_info(const application_info& app) noexcept -> instance_create_info& {
        native_.pApplicationInfo = &app.native();
        return *this;
    }

    auto layers(const std::span<const char* const> vec) noexcept -> instance_create_info& {
        native_.enabledLayerCount   = static_cast<u32>(vec.size());
        native_.ppEnabledLayerNames = vec.data();
        return *this;
    }

    auto extensions(const std::span<const char* const> vec) noexcept -> instance_create_info& {
        native_.enabledExtensionCount   = static_cast<u32>(vec.size());
        native_.ppEnabledExtensionNames = vec.data();
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkInstanceCreateInfo& { return native_; }

  private:
    VkInstanceCreateInfo native_{};
};
}  // namespace hgn