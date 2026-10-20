#pragma once

#include <cassert>
#include <type_traits>
#include <utility>

#include <GLFW/glfw3.h>

#include "kgm/enums.hpp"
#include "kgm/window.hpp"

namespace kgm {
template <typename F>
    requires std::is_nothrow_invocable_r_v<void, F, window&, key, action>
inline void set_key_callback(window& win, F&& callback) noexcept {
    static thread_local constinit auto fn_opt = std::optional<F>{};
    fn_opt                                    = std::forward<F>(callback);

    glfwSetKeyCallback(
        &win,
        [](window*              win_ptr,
           int                  button,
           [[maybe_unused]] int scancode,
           int                  act,
           [[maybe_unused]] int mods) noexcept -> void {
            assert(win_ptr != nullptr);
            if (fn_opt == std::nullopt) return;

            auto& fn = *fn_opt;
            fn(*win_ptr, static_cast<key>(button), static_cast<action>(act));
        }
    );
}

template <typename F>
    requires std::is_nothrow_invocable_r_v<void, F, window&, float, float>
inline void set_cursor_pos_callback(window& win, F&& callback) noexcept {
    static thread_local constinit auto fn_opt = std::optional<F>{};
    fn_opt                                    = std::forward<F>(callback);

    glfwSetCursorPosCallback(&win, [](window* win_ptr, double xpos, double ypos) noexcept -> void {
        assert(win_ptr != nullptr);
        if (fn_opt == std::nullopt) return;

        auto& fn = *fn_opt;
        fn(*win_ptr, static_cast<float>(xpos), static_cast<float>(ypos));
    });
}

template <typename F>
    requires std::is_nothrow_invocable_r_v<void, F, window&, mouse_button, action>
inline void set_mouse_button_callback(window& win, F&& callback) noexcept {
    static thread_local constinit auto* fn_opt = std::optional<F>{};
    fn_opt                                     = std::forward<F>(callback);

    glfwSetMouseButtonCallback(
        &win, [](window* win_ptr, int button, int act, [[maybe_unused]] int mods) noexcept -> void {
            assert(win_ptr != nullptr);
            if (fn_opt == nullptr) return;

            auto& fn = *fn_opt;
            fn(*win_ptr, static_cast<mouse_button>(button), static_cast<action>(act));
        }
    );
}

template <typename F>
    requires std::is_nothrow_invocable_r_v<void, F, window&, std::uint32_t, std::uint32_t>
inline void set_window_size_callback(window& win, F&& callback) noexcept {
    static thread_local auto fn_opt = std::optional<F>{};
    fn_opt                          = std::forward<F>(callback);

    glfwSetWindowSizeCallback(&win, [](window* win_ptr, int width, int height) noexcept -> void {
        assert(win_ptr != nullptr);
        if (fn_opt == std::nullopt) return;

        auto& fn = *fn_opt;
        fn(*win_ptr, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
    });
}
}  // namespace kgm