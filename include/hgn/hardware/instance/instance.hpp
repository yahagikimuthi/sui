#pragma once

#include <expected>
#include <functional>
#include <ranges>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/detail/hardware/instance/instance_impl.hpp"
#include "hgn/enums.hpp"
#include "hgn/hardware/instance/info.hpp"

namespace hgn {
using instance = detail::instance_impl;

[[nodiscard]] inline auto try_make_instance(const instance_create_info& info) noexcept
    -> std::expected<std::reference_wrapper<instance>, result>;

inline void destroy_instance(instance& target) noexcept;

class layer_properties final {
  public:
    explicit layer_properties(const detail::layer_properties_impl& impl) noexcept;

    [[nodiscard]] auto name() const noexcept -> std::string_view;

  private:
    detail::layer_properties_impl impl_;
};

[[nodiscard]] inline auto try_enumerate_instance_layer_properties() noexcept
    -> std::expected<std::vector<layer_properties>, result>;
}  // namespace hgn

// ==================================================
// Implementation details only below here
// ==================================================

namespace hgn {
[[nodiscard]] inline auto try_make_instance(const instance_create_info& info) noexcept
    -> std::expected<std::reference_wrapper<instance>, result> {
    return detail::try_make_instance_impl(info);
}

inline void destroy_instance(instance& target) noexcept {
    detail::destroy_instance_impl(target);
}

inline layer_properties::layer_properties(const detail::layer_properties_impl& impl) noexcept
    : impl_{impl} {}

[[nodiscard]] inline auto layer_properties::name() const noexcept -> std::string_view {
    return impl_.name();
}

[[nodiscard]] inline auto try_enumerate_instance_layer_properties() noexcept
    -> std::expected<std::vector<layer_properties>, result> {
    auto props = detail::try_enumerate_instance_layer_properties_impl();
    if (not props) return std::unexpected{props.error()};
    return *props |
           std::views::transform(
               [](const detail::layer_properties_impl& impl) noexcept -> layer_properties {
                   return layer_properties{impl};
               }
           ) |
           std::ranges::to<std::vector>();
}
}  // namespace hgn