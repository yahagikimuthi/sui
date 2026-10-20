#pragma once

#include <GLFW/glfw3.h>

#include "kgm/enums.hpp"
#include "kgm/key.hpp"
#include "kgm/window.hpp"

namespace kgm::detail {
class callback_manager final {
    using key_t        = void(window&, const key, const action);
    using cursor_pos_t = void(window&, const float, const float);

  public:
    static void set_keys(window& win, key_t& callback) noexcept {
        key_ = &callback;
        glfwSetKeyCallback(
            &win,
            [](GLFWwindow*                win,
               const int                  button,
               [[maybe_unused]] const int scancode,
               const int                  act,
               [[maybe_unused]] const int mods) noexcept -> void {
                if (win == nullptr or key_ == nullptr) return;
                key_(*win, static_cast<key>(button), static_cast<action>(act));
            }
        );
    }

    static void set_cursor_pos(window& win, cursor_pos_t& callback) noexcept {
        cursor_pos_ = &callback;
        glfwSetCursorPosCallback(
            &win, [](GLFWwindow* win, const double xpos, const double ypos) noexcept -> void {
                if (win == nullptr or cursor_pos_ == nullptr) return;
                cursor_pos_(*win, static_cast<float>(xpos), static_cast<float>(ypos));
            }
        );
    }

  private:
    static inline constinit auto* key_        = static_cast<key_t*>(nullptr);
    static inline constinit auto* cursor_pos_ = static_cast<cursor_pos_t*>(nullptr);
};

}  // namespace kgm::detail

namespace kgm {
inline void set_key_callback(
    window& win, void (&callback)(window&, const key, const action)
) noexcept {
    detail::callback_manager::set_keys(win, callback);
}

inline void set_cursor_pos_callback(
    window& win, void (&callback)(window&, const float, const float)
) noexcept {
    detail::callback_manager::set_cursor_pos(win, callback);
}
}  // namespace kgm