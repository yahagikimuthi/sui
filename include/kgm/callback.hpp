#pragma once

#include <GLFW/glfw3.h>

#include "kgm/enums.hpp"
#include "kgm/key.hpp"
#include "kgm/window.hpp"

namespace kgm::detail {
class callback_manager final {
    using key_t = void(window&, const key, const action);

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
                if (key_ == nullptr) return;
                if (win == nullptr) return;
                key_(*win, static_cast<key>(button), static_cast<action>(act));
            }
        );
    }

  private:
    static inline constinit auto* key_ = static_cast<key_t*>(nullptr);
};

}  // namespace kgm::detail

namespace kgm {
inline void set_key_callback(
    window& win, void (&callback)(window&, const key, const action)
) noexcept {
    detail::callback_manager::set_keys(win, callback);
}
}  // namespace kgm