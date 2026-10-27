#pragma once

#include <cstdint>
#include <string_view>

#include <vulkan/vulkan.h>

#include "hgn/detail/hardware/instance/app_info_impl.hpp"

namespace hgn {
class application_info final {
  public:
    explicit application_info() noexcept = default;

    auto app_name(const std::string_view name) noexcept -> application_info&;

    auto app_version(const std::uint32_t major, std::uint32_t minor, std::uint32_t patch) noexcept
        -> application_info&;

    auto engine_name(const std::string_view name) noexcept -> application_info&;

    auto engine_version(
        const std::uint32_t major, std::uint32_t minor, std::uint32_t patch
    ) noexcept -> application_info&;

    auto api_version(const std::uint32_t major, std::uint32_t minor, std::uint32_t patch) noexcept
        -> application_info&;

    [[nodiscard]] auto get() const noexcept -> const VkApplicationInfo&;

  private:
    detail::application_info_impl impl_;
};
}  // namespace hgn

// ==================================================
// Implementation details only below here
// ==================================================

namespace hgn {
inline auto application_info::app_name(const std::string_view name) noexcept -> application_info& {
    impl_.app_name(name);
    return *this;
}

inline auto application_info::app_version(
    const std::uint32_t major, std::uint32_t minor, std::uint32_t patch
) noexcept -> application_info& {
    impl_.app_version(major, minor, patch);
    return *this;
}

inline auto application_info::engine_name(const std::string_view name) noexcept
    -> application_info& {
    impl_.engine_name(name);
    return *this;
}

inline auto application_info::engine_version(
    const std::uint32_t major, std::uint32_t minor, std::uint32_t patch
) noexcept -> application_info& {
    impl_.engine_version(major, minor, patch);
    return *this;
}

inline auto application_info::api_version(
    const std::uint32_t major, std::uint32_t minor, std::uint32_t patch
) noexcept -> application_info& {
    impl_.api_version(major, minor, patch);
    return *this;
}
}  // namespace hgn