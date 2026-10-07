#pragma once

#include <GLFW/glfw3.h>
#include <expected>
#include <vector>

#include "kgm/types.hpp"

namespace kgm {
[[nodiscard]] inline auto try_init() noexcept -> bool {
    const auto error_code = glfwInit();
    return (error_code == GLFW_TRUE);
}

[[nodiscard]] inline auto get_required_instance_extensions() -> std::vector<const char*> {
    auto         count          = u32{};
    const char** extensions_ptr = glfwGetRequiredInstanceExtensions(&count);

    if (extensions_ptr == nullptr or count == 0) {
        return {};
    }

    // ポインタ範囲指定コンストラクタで vector にまとめて返す
    return {extensions_ptr, extensions_ptr + count};  // NOLINT
}
}  // namespace kgm