#pragma once

#include <GLFW/glfw3.h>

#include "kgm/enums.hpp"
#include "kgm/key.hpp"
#include "kgm/window.hpp"

namespace kgm::detail {
class callback_manager final {
    using key_t          = void(window&, key, action);
    using cursor_pos_t   = void(window&, float, float);
    using mouse_button_t = void(window&, mouse_button, action);

  public:
    static void set(window& win, key_t& callback) noexcept {
        key_ = &callback;
        glfwSetKeyCallback(
            &win,
            [](GLFWwindow*          win,
               int                  button,
               [[maybe_unused]] int scancode,
               int                  act,
               [[maybe_unused]] int mods) noexcept -> void {
                if (win == nullptr or key_ == nullptr) return;
                key_(*win, static_cast<key>(button), static_cast<action>(act));
            }
        );
    }

    static void set(window& win, cursor_pos_t& callback) noexcept {
        cursor_pos_ = &callback;
        glfwSetCursorPosCallback(
            &win, [](GLFWwindow* win, double xpos, double ypos) noexcept -> void {
                if (win == nullptr or cursor_pos_ == nullptr) return;
                cursor_pos_(*win, static_cast<float>(xpos), static_cast<float>(ypos));
            }
        );
    }

    static void set(window& win, mouse_button_t& callback) noexcept {
        mouse_button_ = &callback;
        glfwSetMouseButtonCallback(
            &win,
            [](GLFWwindow* win, int button, int act, [[maybe_unused]] int mods) noexcept -> void {
                if (win == nullptr or mouse_button_ == nullptr) return;
                mouse_button_(*win, static_cast<mouse_button>(button), static_cast<action>(act));
            }
        );
    }

  private:
    static inline constinit auto* key_          = static_cast<key_t*>(nullptr);
    static inline constinit auto* cursor_pos_   = static_cast<cursor_pos_t*>(nullptr);
    static inline constinit auto* mouse_button_ = static_cast<mouse_button_t*>(nullptr);
};

}  // namespace kgm::detail

namespace kgm {
inline void set_key_callback(window& win, void (&callback)(window&, key, action)) noexcept {
    detail::callback_manager::set(win, callback);
}

inline void set_cursor_pos_callback(window& win, void (&callback)(window&, float, float)) noexcept {
    detail::callback_manager::set(win, callback);
}

inline void set_mouse_button_callback(
    window& win, void (&callback)(window&, mouse_button, action)
) noexcept {
    detail::callback_manager::set(win, callback);
}
}  // namespace kgm