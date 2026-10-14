#pragma once

#include <cstdint>
#include <ranges>
#include <string_view>
#include <vector>

#include <GLFW/glfw3.h>

namespace kgm {
[[nodiscard]] inline auto try_init() noexcept -> bool {
    const auto error_code = glfwInit();
    return (error_code == GLFW_TRUE);
}

[[nodiscard]] inline auto get_required_instance_extensions() noexcept
    -> std::vector<std::string_view> {
    auto         count          = std::uint32_t{};
    auto         extensions     = std::vector<const char*>{};
    const auto** extensions_ptr = glfwGetRequiredInstanceExtensions(&count);

    if (extensions_ptr == nullptr or count == 0) {
        return std::vector<std::string_view>{};
    }

    // ポインタ範囲指定コンストラクタで vector にまとめる
    extensions = std::vector<const char*>(extensions_ptr, extensions_ptr + count);  // NOLINT

    return extensions | std::views::transform([](const char* ptr) noexcept -> std::string_view {
               return std::string_view{ptr};
           }) |
           std::ranges::to<std::vector>();
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