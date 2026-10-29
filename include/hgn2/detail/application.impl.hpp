#pragma once

#ifndef HGN_DETAIL_APPLICATION_IMPL_HPP_
#include "hgn2/application.hpp"
#endif

#include <cstdint>
#include <string_view>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn {
inline application::application(const application& other) noexcept
    : info_{other.info_}, name_{other.name_}, engine_name_{other.engine_name_} {
    reset();
}

inline auto application::operator=(const application& other) noexcept -> application& {
    if (this == &other) return *this;

    info_        = other.info_;
    name_        = other.name_;
    engine_name_ = other.engine_name_;
    reset();

    return *this;
}

inline application::application(application&& other) noexcept
    : info_{other.info_},
      name_{std::move(other.name_)},
      engine_name_{std::move(other.engine_name_)} {
    reset();
}

inline auto application::operator=(application&& other) noexcept -> application& {
    if (this == &other) return *this;

    info_        = other.info_;
    name_        = std::move(other.name_);
    engine_name_ = std::move(other.engine_name_);

    reset();
    return *this;
}

inline auto application::builder::app_name(const std::string_view name) noexcept -> builder& {
    name_ = name;
    return *this;
}

inline auto application::builder::app_version(
    const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
) noexcept -> builder& {
    app_version_ = VK_MAKE_VERSION(major, minor, patch);
    return *this;
}

inline auto application::builder::engine_name(const std::string_view name) noexcept -> builder& {
    engine_name_ = name;
    return *this;
}

inline auto application::builder::engine_version(
    const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
) noexcept -> builder& {
    engine_version_ = VK_MAKE_VERSION(major, minor, patch);
    return *this;
}

inline application::application(const builder& builder) noexcept
    : name_{builder.name_}, engine_name_{builder.engine_name_} {
    info_.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info_.pApplicationName   = name_.c_str();
    info_.applicationVersion = builder.app_version_;
    info_.pEngineName        = builder.engine_name_.c_str();
    info_.engineVersion      = builder.engine_version_;
    info_.apiVersion         = VK_API_VERSION_1_3;
}

inline void application::reset() noexcept {
    if (info_.pApplicationName != nullptr) {
        info_.pApplicationName = name_.c_str();
    }
    if (info_.pEngineName != nullptr) {
        info_.pEngineName = engine_name_.c_str();
    }
}
}  // namespace hgn