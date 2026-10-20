#pragma once

#include <cstdint>

#include <GLFW/glfw3.h>

namespace kgm {
enum class action : std::int32_t {  // NOLINT
    press   = GLFW_PRESS,
    release = GLFW_RELEASE
};

enum class mouse_button : std::uint8_t {
    num1   = GLFW_MOUSE_BUTTON_1,
    num2   = GLFW_MOUSE_BUTTON_2,
    num3   = GLFW_MOUSE_BUTTON_3,
    num4   = GLFW_MOUSE_BUTTON_4,
    num5   = GLFW_MOUSE_BUTTON_5,
    num6   = GLFW_MOUSE_BUTTON_6,
    num7   = GLFW_MOUSE_BUTTON_7,
    num8   = GLFW_MOUSE_BUTTON_8,
    last   = GLFW_MOUSE_BUTTON_LAST,
    left   = GLFW_MOUSE_BUTTON_LEFT,
    right  = GLFW_MOUSE_BUTTON_RIGHT,
    middle = GLFW_MOUSE_BUTTON_MIDDLE
};
}  // namespace kgm