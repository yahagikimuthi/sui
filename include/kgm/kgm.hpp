#pragma once

#include <cassert>

#include <GLFW/glfw3.h>

namespace kgm {
[[nodiscard]] inline auto try_init() noexcept -> bool {
    return glfwInit() != GLFW_FALSE;
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