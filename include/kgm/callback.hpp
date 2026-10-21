#pragma once

#include <cassert>
#include <concepts>
#include <utility>

#include <GLFW/glfw3.h>

#include "kgm/enums.hpp"
#include "kgm/window.hpp"

namespace kgm {
template <std::convertible_to<void (*)(window&, key, action)> F>
inline void set_key_callback(window& win, F&& callback) noexcept {
    static constinit auto fn = std::forward<F>(callback);

    glfwSetKeyCallback(
        &win,
        [](window*              win_ptr,
           int                  button,
           [[maybe_unused]] int scancode,
           int                  act,
           [[maybe_unused]] int mods) noexcept -> void {
            assert(win_ptr != nullptr);
            fn(*win_ptr, static_cast<key>(button), static_cast<action>(act));
        }
    );
}

template <std::convertible_to<void (*)(window&, float, float)> F>
inline void set_cursor_pos_callback(window& win, F&& callback) noexcept {
    static constinit auto fn = std::forward<F>(callback);

    glfwSetCursorPosCallback(&win, [](window* win_ptr, double xpos, double ypos) noexcept -> void {
        assert(win_ptr != nullptr);
        fn(*win_ptr, static_cast<float>(xpos), static_cast<float>(ypos));
    });
}

template <std::convertible_to<void (*)(window&, mouse_button, action)> F>
inline void set_mouse_button_callback(window& win, F&& callback) noexcept {
    static constinit auto fn = std::forward<F>(callback);

    glfwSetMouseButtonCallback(
        &win, [](window* win_ptr, int button, int act, [[maybe_unused]] int mods) noexcept -> void {
            assert(win_ptr != nullptr);
            fn(*win_ptr, static_cast<mouse_button>(button), static_cast<action>(act));
        }
    );
}

template <std::convertible_to<void (*)(window&, std::uint32_t, std::uint32_t)> F>
inline void set_window_size_callback(window& win, F&& callback) noexcept {
    static constinit auto fn = std::forward<F>(callback);

    glfwSetWindowSizeCallback(&win, [](window* win_ptr, int width, int height) noexcept -> void {
        assert(win_ptr != nullptr);
        fn(*win_ptr, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
    });
}
}  // namespace kgm