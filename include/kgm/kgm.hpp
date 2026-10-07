#pragma once

#include <GLFW/glfw3.h>
#include <expected>

#include "kgm/types.hpp"

namespace kgm {
[[nodiscard]] inline auto try_init() noexcept -> std::expected<void, i32> {
    const auto error_code = glfwInit();
    if (error_code != 0) return std::unexpected{error_code};

    return {};
}
}  // namespace kgm