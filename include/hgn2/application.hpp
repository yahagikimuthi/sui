#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <vulkan/vulkan.h>

namespace hgn {
class application final {
  public:
    application(const application& other) noexcept;
    auto operator=(const application& other) noexcept -> application&;
    application(application&& other) noexcept;
    auto operator=(application&& other) noexcept -> application&;
    ~application() noexcept = default;

    class builder final {
        friend class application;

      public:
        explicit builder() noexcept = default;

        auto app_name(const std::string_view name) noexcept -> builder&;

        auto app_version(
            const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
        ) noexcept -> builder&;

        auto engine_name(const std::string_view name) noexcept -> builder&;

        auto engine_version(
            const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
        ) noexcept -> builder&;

      private:
        std::string   name_;
        std::uint32_t app_version_{};
        std::string   engine_name_;
        std::uint32_t engine_version_{};
    };

  private:
    friend class application::builder;

    explicit application(const builder& builder) noexcept;

    void reset() noexcept;

    VkApplicationInfo info_{};
    std::string       name_;
    std::string       engine_name_;
};
}  // namespace hgn

#define HGN_DETAIL_APPLICATION_INL_
#include "hgn2/detail/application.impl.hpp"
#undef HGN_DETAIL_APPLICATION_INL_