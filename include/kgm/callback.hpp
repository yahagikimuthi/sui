#pragma once

#include <GLFW/glfw3.h>

#include "kgm/enums.hpp"
#include "kgm/window.hpp"

namespace kgm {
inline void set_key_callback(window& win, void (*callback)(window&, key, action)) noexcept {
    static thread_local constinit auto* fn = static_cast<void (*)(window&, key, action)>(nullptr);
    fn                                     = callback;

    glfwSetKeyCallback(
        &win,
        [](window*              win_ptr,
           int                  button,
           [[maybe_unused]] int scancode,
           int                  act,
           [[maybe_unused]] int mods) noexcept -> void {
            if (fn != nullptr) fn(*win_ptr, static_cast<key>(button), static_cast<action>(act));
        }
    );
}

inline void set_cursor_pos_callback(window& win, void (*callback)(window&, float, float)) noexcept {
    static thread_local constinit auto* fn = static_cast<void (*)(window&, float, float)>(nullptr);
    fn                                     = callback;
    glfwSetCursorPosCallback(&win, [](window* win_ptr, double xpos, double ypos) noexcept -> void {
        if (fn != nullptr) fn(*win_ptr, static_cast<float>(xpos), static_cast<float>(ypos));
    });
}

inline void set_mouse_button_callback(
    window& win, void (*callback)(window&, mouse_button, action)
) noexcept {
    static thread_local constinit auto* fn =
        static_cast<void (*)(window&, mouse_button, action)>(nullptr);
    fn = callback;

    glfwSetMouseButtonCallback(
        &win, [](window* win_ptr, int button, int act, [[maybe_unused]] int mods) noexcept -> void {
            if (fn != nullptr)
                fn(*win_ptr, static_cast<mouse_button>(button), static_cast<action>(act));
        }
    );
}

inline void set_window_size_callback(
    window& win, void (*callback)(window&, std::uint32_t, std::uint32_t)
) noexcept {
    static thread_local auto* fn =
        static_cast<void (*)(window&, std::uint32_t, std::uint32_t)>(nullptr);
    fn = callback;

    glfwSetWindowSizeCallback(&win, [](window* win_ptr, int width, int height) noexcept -> void {
        if (fn != nullptr)
            fn(*win_ptr, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
    });
}
}  // namespace kgm