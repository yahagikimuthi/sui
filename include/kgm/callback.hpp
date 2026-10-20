#pragma once

#include <GLFW/glfw3.h>

#include "kgm/enums.hpp"
#include "kgm/window.hpp"

// TODO コールバックについて、呼ばれた際にWindowがヌルである場合があるか調査
// TODO ヌルである場合はoptional<T&>の利用、ヌルでない場合は参照を受ける関数を扱うものとする
// TODO 関数ポインタをスレッドローカル指定した場合の安全性, テスト追加
namespace kgm::detail {
class [[deprecated]] callback_manager final {
    using key_fn          = void(window&, key, action);
    using cursor_pos_fn   = void(window&, float, float);
    using mouse_button_fn = void(window&, mouse_button, action);

  public:
    static void set(window& win, key_fn* callback) noexcept {
        key_ = callback;
        glfwSetKeyCallback(
            &win,
            [](GLFWwindow*          win_ptr,
               int                  button,
               [[maybe_unused]] int scancode,
               int                  act,
               [[maybe_unused]] int mods) noexcept -> void {
                if (win_ptr == nullptr or key_ == nullptr) return;
                key_(*win_ptr, static_cast<key>(button), static_cast<action>(act));
            }
        );
    }

    static void set(window& win, cursor_pos_fn* callback) noexcept {
        cursor_pos_ = callback;
        glfwSetCursorPosCallback(
            &win, [](GLFWwindow* win_ptr, double xpos, double ypos) noexcept -> void {
                if (win_ptr == nullptr or cursor_pos_ == nullptr) return;
                cursor_pos_(*win_ptr, static_cast<float>(xpos), static_cast<float>(ypos));
            }
        );
    }

    static void set(window& win, mouse_button_fn* callback) noexcept {
        mouse_button_ = callback;
        glfwSetMouseButtonCallback(
            &win,
            [](
                GLFWwindow* win_ptr, int button, int act, [[maybe_unused]] int mods
            ) noexcept -> void {
                if (win_ptr == nullptr or mouse_button_ == nullptr) return;
                mouse_button_(
                    *win_ptr, static_cast<mouse_button>(button), static_cast<action>(act)
                );
            }
        );
    }

  private:
    static inline constinit auto* key_          = static_cast<key_fn*>(nullptr);
    static inline constinit auto* cursor_pos_   = static_cast<cursor_pos_fn*>(nullptr);
    static inline constinit auto* mouse_button_ = static_cast<mouse_button_fn*>(nullptr);
};
}  // namespace kgm::detail

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