#pragma once

#ifndef HGN_DETAIL_WINDOW_IMPL_HPP
#include "hgn2/window.hpp"
#endif

#include <cassert>
#include <expected>

#include "hgn2/enums.hpp"
#include "hgn2/instance.hpp"
#include "hgn2/surface.hpp"

namespace hgn {
inline auto window::make_surface(instance& instance_ref) noexcept
    -> std::expected<surface_khr, result> {
    auto*      surface = static_cast<VkSurfaceKHR_T*>(nullptr);
    const auto res     = glfwCreateWindowSurface(&instance_ref.get(), &get(), nullptr, &surface);
    if (res != VK_SUCCESS) return make_unexpected(res);

    assert(surface != nullptr);
    return surface_khr{*surface};
}
}  // namespace hgn