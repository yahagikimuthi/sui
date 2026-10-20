#pragma once

#include <cstdint>

#include <GLFW/glfw3.h>

namespace kgm {
enum class action : std::int32_t {  // NOLINT
    press   = GLFW_PRESS,
    release = GLFW_RELEASE
};
}  // namespace kgm