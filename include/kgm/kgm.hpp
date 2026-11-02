#pragma once

#include <cassert>
#include "kgm/detail/window.hpp"

#include <GLFW/glfw3.h>

namespace kgm {
[[nodiscard]] inline auto init() noexcept -> bool {
    return glfwInit() != GLFW_FALSE;
}

[[nodiscard]] inline auto get_window_creator() noexcept -> window_creator {
    return window_creator{};
}

inline void poll_events() noexcept {
    glfwPollEvents();
}

inline void terminate() noexcept {
    glfwTerminate();
}

static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);
}  // namespace kgm