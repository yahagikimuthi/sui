#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>

namespace kgm {
[[nodiscard]] inline auto try_init() noexcept -> bool {
    const auto error_code = glfwInit();
    return (error_code == GLFW_TRUE);
}

[[nodiscard]] inline auto get_required_instance_extensions() noexcept -> std::vector<const char*> {
    auto         count          = std::uint32_t{};
    auto         extensions     = std::vector<const char*>{};
    const auto** extensions_ptr = glfwGetRequiredInstanceExtensions(&count);

    if (extensions_ptr == nullptr or count == 0) {
        return extensions;
    }

    // ポインタ範囲指定コンストラクタで vector にまとめて返す
    extensions = std::vector<const char*>(extensions_ptr, extensions_ptr + count);  // NOLINT
    return extensions;
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