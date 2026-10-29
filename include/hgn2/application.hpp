#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn {
class application final {
  public:
    application(const application& other) noexcept
        : info_{other.info_}, name_{other.name_}, engine_name_{other.engine_name_} {
        reset();
    }

    auto operator=(const application& other) noexcept -> application& {
        if (this == &other) return *this;

        info_        = other.info_;
        name_        = other.name_;
        engine_name_ = other.engine_name_;
        reset();

        return *this;
    }

    application(application&& other) noexcept
        : info_{other.info_},
          name_{std::move(other.name_)},
          engine_name_{std::move(other.engine_name_)} {
        reset();
    }

    auto operator=(application&& other) noexcept -> application& {
        if (this == &other) return *this;

        info_        = other.info_;
        name_        = std::move(other.name_);
        engine_name_ = std::move(other.engine_name_);

        reset();
        return *this;
    }

    ~application() noexcept = default;

    [[nodiscard]] auto info() const noexcept -> const VkApplicationInfo& { return info_; }

    class builder final {
        friend class application;

      public:
        explicit builder() noexcept = default;

        auto app_name(const std::string_view name) noexcept -> builder& {
            name_ = name;
            return *this;
        }

        auto app_version(
            const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
        ) noexcept -> builder& {
            app_version_ = VK_MAKE_VERSION(major, minor, patch);
            return *this;
        }

        auto engine_name(const std::string_view name) noexcept -> builder& {
            engine_name_ = name;
            return *this;
        }

        auto engine_version(
            const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
        ) noexcept -> builder& {
            engine_version_ = VK_MAKE_VERSION(major, minor, patch);
            return *this;
        }

        [[nodiscard]] auto build() noexcept -> application { return application{*this}; }

      private:
        std::string   name_;
        std::uint32_t app_version_{};
        std::string   engine_name_;
        std::uint32_t engine_version_{};
    };

  private:
    friend class application::builder;

    explicit application(const builder& builder) noexcept
        : name_{builder.name_}, engine_name_{builder.engine_name_} {
        info_.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        info_.pApplicationName   = name_.c_str();
        info_.applicationVersion = builder.app_version_;
        info_.pEngineName        = builder.engine_name_.c_str();
        info_.engineVersion      = builder.engine_version_;
        info_.apiVersion         = VK_API_VERSION_1_3;
    }

    void reset() noexcept {
        if (info_.pApplicationName != nullptr) {
            info_.pApplicationName = name_.c_str();
        }
        if (info_.pEngineName != nullptr) {
            info_.pEngineName = engine_name_.c_str();
        }
    }

    VkApplicationInfo info_{};
    std::string       name_;
    std::string       engine_name_;
};
}  // namespace hgn