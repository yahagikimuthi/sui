#pragma once

#include <vulkan/vulkan.h>
#include <string_view>

#include "hgn/detail/others.hpp"
#include "hgn/types.hpp"
#include "hgn/version.hpp"

namespace hgn {
class application_info final {
  public:
    explicit application_info() noexcept = default;

    template <typename T>
        requires detail::is_string_literal_v<T>
    auto application_name(const T& name) noexcept -> application_info& {
        application_name_ = name;
        return *this;
    }

    template <typename T>
    auto engine_name(const T& name) noexcept -> application_info& {
        engine_name_ = name;
        return *this;
    }

    auto application_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        application_version_ = {.major = major, .minor = minor, .patch = patch};
        return *this;
    }

    auto application_version(const version_t& version) noexcept -> application_info& {
        return application_version(version.major, version.minor, version.patch);
    }

    auto engine_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        engine_version_ = {.major = major, .minor = minor, .patch = patch};
        return *this;
    }

    auto engine_version(const version_t& version) noexcept -> application_info& {
        return engine_version(version.major, version.minor, version.patch);
    }

    auto api_version(const u32 variant, const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        api_version_ = {.variant = variant, .major = major, .minor = minor, .patch = patch};
        return *this;
    }

    auto api_version(const u32 major, const u32 minor, const u32 patch) noexcept
        -> application_info& {
        return api_version(0, major, minor, patch);
    }

    auto api_version(const api_version_t& version) noexcept -> application_info& {
        return api_version(version.variant, version.major, version.minor, version.patch);
    }

    [[nodiscard]] auto application_name() const noexcept -> std::string_view {
        return application_name_;
    }
    [[nodiscard]] auto engine_name() const noexcept -> std::string_view { return engine_name_; }
    [[nodiscard]] auto application_version() const noexcept -> version_t {
        return application_version_;
    }
    [[nodiscard]] auto engine_version() const noexcept -> version_t { return engine_version_; }
    [[nodiscard]] auto api_version() const noexcept -> api_version_t { return api_version_; }

  private:
    std::string_view application_name_;
    std::string_view engine_name_;
    version_t        application_version_{};
    version_t        engine_version_{};
    api_version_t    api_version_{};
};
}  // namespace hgn