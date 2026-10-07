#pragma once

#include <GLFW/glfw3.h>
#include <optional>
#include <string>
#include <string_view>

#include "kgm/types.hpp"

namespace kgm {
enum class window_hint_t : u8 {
    client_api,
    no_api,
};

using window_hint_t::client_api;
using window_hint_t::no_api;

void inline window_hint(const window_hint_t hint1, const window_hint_t hint2) noexcept {
    glfwWindowHint(static_cast<i32>(hint1), static_cast<i32>(hint2));
}

using window = GLFWwindow;

void inline destroy_window(window& target) noexcept {
    glfwDestroyWindow(&target);
}

void inline terminate() noexcept {
    glfwTerminate();
}

auto inline create_window(const u32 width, const u32 height, std::string_view title) noexcept
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
}  // namespace kgm