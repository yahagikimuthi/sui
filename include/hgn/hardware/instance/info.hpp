#pragma once

#include <span>
#include <string_view>

#include <vulkan/vulkan.h>

#include "hgn/detail/hardware/instance/info_impl.hpp"
#include "hgn/hardware/instance/app_info.hpp"

namespace hgn {
class instance_create_info final {
  public:
    explicit instance_create_info() noexcept = default;

    auto app_info(const application_info& app) noexcept -> instance_create_info&;

    auto layer(const std::string_view name) noexcept -> instance_create_info&;

    auto extensions(const std::span<const std::string_view> names) noexcept
        -> instance_create_info&;

    [[nodiscard]] auto get() const noexcept -> const VkInstanceCreateInfo&;

  private:
    detail::instance_create_info_impl impl_;
};
}  // namespace hgn

// ==================================================
// Implementation details only below here
// ==================================================

namespace hgn {
inline auto instance_create_info::app_info(const application_info& app) noexcept
    -> instance_create_info& {
    impl_.app_info(app);
    return *this;
}

inline auto instance_create_info::layer(const std::string_view name) noexcept
    -> instance_create_info& {
    impl_.layer(name);
    return *this;
}

inline auto instance_create_info::extensions(const std::span<const std::string_view> names) noexcept
    -> instance_create_info& {
    impl_.extensions(names);
    return *this;
}

inline auto instance_create_info::get() const noexcept -> const VkInstanceCreateInfo& {
    return impl_.get();
}
}  // namespace hgn