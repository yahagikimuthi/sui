#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <vulkan/vulkan.h>
#include <hgn/instance.hpp>
#include <hgn/surface.hpp>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

namespace kgm {
enum class window_hint : std::int32_t { client_api = GLFW_CLIENT_API };

enum class window_value : std::int32_t { no_api = GLFW_NO_API };  // NOLINT

inline void set_window_hint(const window_hint hint, const window_value value) noexcept {
    glfwWindowHint(std::to_underlying(hint), std::to_underlying(value));
}

inline void set_default_window_hints() noexcept {
    glfwDefaultWindowHints();
}

using window = GLFWwindow;

inline void destroy_window(window& target) noexcept {
    glfwDestroyWindow(&target);
}

[[nodiscard]] inline auto try_make_window(
    const std::uint32_t width, const std::uint32_t height, const std::string_view title
) noexcept -> std::optional<window&> {
    auto* ptr = glfwCreateWindow(
        static_cast<std::int32_t>(width),
        static_cast<std::int32_t>(height),
        std::string{title}.c_str(),
        nullptr,
        nullptr
    );
    if (ptr == nullptr) return std::nullopt;
    return *ptr;
}

[[nodiscard]] inline auto should_window_close(window& win) noexcept -> bool {
    return glfwWindowShouldClose(&win) != 0;
}

template <typename T>
    requires std::is_class_v<T>
inline void set_window_user(window& win, T& user) noexcept {
    glfwSetWindowUserPointer(&win, &user);
}

inline void reset_window_user(window& win) noexcept {
    glfwSetWindowUserPointer(&win, nullptr);
}

template <typename T>
    requires std::is_class_v<std::remove_const_t<T>>
[[nodiscard]] inline auto try_get_window_user(window& win) noexcept -> std::optional<T&> {
    auto* user = static_cast<T*>(glfwGetWindowUserPointer(&win));
    if (user == nullptr) return std::nullopt;
    return *user;
}

[[nodiscard]] inline auto try_make_window_surface(hgn::instance& instance_ref, window& win) noexcept
    -> std::expected<std::reference_wrapper<hgn::surface_khr>, hgn::result> {
    auto*      surface = static_cast<hgn::surface_khr*>(nullptr);
    const auto result  = glfwCreateWindowSurface(&instance_ref, &win, nullptr, &surface);
    if (result != VK_SUCCESS) return hgn::make_result(result);

    assert(surface != nullptr);

    return std::ref(*surface);
}
}  // namespace kgm