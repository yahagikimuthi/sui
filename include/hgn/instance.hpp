#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/declaration.hpp"
#include "hgn/detail/others.hpp"
#include "hgn/enums.hpp"

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

class instance_create_info final {
  public:
    explicit instance_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    }

    instance_create_info(const instance_create_info& other) noexcept
        : native_{other.native_},
          app_info_{other.app_info_},
          layer_{other.layer_},
          extensions_{other.extensions_} {
        reset();
    }

    auto operator=(const instance_create_info& other) noexcept -> instance_create_info& {
        if (this == &other) return *this;

        native_     = other.native_;
        app_info_   = other.app_info_;
        layer_      = other.layer_;
        extensions_ = other.extensions_;

        reset();
        return *this;
    }

    instance_create_info(instance_create_info&& other) noexcept
        : native_{other.native_},
          app_info_{other.app_info_},
          layer_{other.layer_},
          extensions_{std::move(other.extensions_)} {
        reset();
    }

    auto operator=(instance_create_info&& other) noexcept -> instance_create_info& {
        if (this == &other) return *this;

        native_     = other.native_;
        app_info_   = other.app_info_;
        layer_      = other.layer_;
        extensions_ = std::move(other.extensions_);

        reset();
        return *this;
    }

    ~instance_create_info() noexcept = default;

    auto app_info(const application_info& app) noexcept -> instance_create_info& {
        app_info_                = app;
        native_.pApplicationInfo = &app_info_.get();
        return *this;
    }

    auto layer(const std::string_view vec) noexcept -> instance_create_info& {
        layer_                      = vec.data();
        native_.enabledLayerCount   = 1;
        native_.ppEnabledLayerNames = &layer_;
        return *this;
    }

    auto extensions(const std::span<const std::string_view> vec) noexcept -> instance_create_info& {
        extensions_.reserve(vec.size());
        extensions_.assign_range(
            vec | std::views::transform([](const std::string_view view) noexcept -> const char* {
                return view.data();
            })
        );
        native_.enabledExtensionCount   = static_cast<std::uint32_t>(extensions_.size());
        native_.ppEnabledExtensionNames = extensions_.data();
        return *this;
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

[[nodiscard]] inline auto try_create_instance(const instance_create_info& info) noexcept
    -> std::expected<std::reference_wrapper<instance>, result> {
    auto*      ins = static_cast<instance*>(nullptr);
    const auto res = vkCreateInstance(&info.get(), nullptr, &ins);
    if (res != VK_SUCCESS) return make_result(res);

    assert(ins != nullptr);
    return std::ref(*ins);
}

inline void destroy_instance(instance& ins) noexcept {
    vkDestroyInstance(&ins, nullptr);
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