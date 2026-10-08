#pragma once

#include <GLFW/glfw3.h>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

#include "kgm/types.hpp"

namespace kgm {
enum class window_hint_t : u8 {
    client_api,
    no_api,
};

using window_hint_t::client_api;
using window_hint_t::no_api;

inline void window_hint(const window_hint_t hint1, const window_hint_t hint2) noexcept {
    glfwWindowHint(static_cast<i32>(hint1), static_cast<i32>(hint2));
}

using window = GLFWwindow;

inline void destroy_window(window& target) noexcept {
    glfwDestroyWindow(&target);
}

inline void terminate() noexcept {
    glfwTerminate();
}

[[nodiscard]] inline auto create_window(
    const u32 width, const u32 height, std::string_view title
) noexcept -> std::optional<window&> {
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

[[nodiscard]] inline auto window_should_close(window& win) noexcept -> bool {
    return glfwWindowShouldClose(&win) != 0;
}

template <typename T>
    requires std::is_class_v<T>
inline void set_window_user_pointer(window& win, T&& user) noexcept {
    glfwSetWindowUserPointer(&win, &user);
}
}  // namespace kgm