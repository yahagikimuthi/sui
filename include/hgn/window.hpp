#pragma once

#include <GLFW/glfw3.h>
#include <optional>
#include <string>
#include <string_view>

#include "hgn/types.hpp"

namespace hgn {
enum class window_hint_t : u8 {
    client_api,
    noe_api,
};

inline constexpr auto client_api = window_hint_t::client_api;
inline constexpr auto no_api     = window_hint_t::noe_api;

void window_hint(const window_hint_t hint1, const window_hint_t hint2) noexcept {
    glfwWindowHint(static_cast<i32>(hint1), static_cast<i32>(hint2));
}

using window = GLFWwindow;

void destroy_window(window& target) noexcept {
    glfwDestroyWindow(&target);
}

void terminate() noexcept {
    glfwTerminate();
}

auto create_window(const u32 width, const u32 height, std::string_view title) noexcept
    -> std::optional<window&> {
    auto* ptr = glfwCreateWindow(
        static_cast<i32>(width),
        static_cast<i32>(height),
        std::string{title}.c_str(),
        nullptr,
        nullptr
    );
    if (ptr == nullptr) return std::nullopt;
    return *ptr;
}
}  // namespace hgn