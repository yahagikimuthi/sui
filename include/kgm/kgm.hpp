#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <GLFW/glfw3.h>

#include "kgm/window.hpp"

namespace kgm {
[[nodiscard]] inline auto init() noexcept -> bool {
    return glfwInit() != 0;
}

class window_creator final {
  public:
    explicit window_creator() noexcept = default;

    auto extent(const std::uint32_t width, const std::uint32_t height) noexcept -> window_creator& {
        width_  = static_cast<int>(width);
        height_ = static_cast<int>(height);
        return *this;
    }

    auto title(const std::string_view title) noexcept -> window_creator& {
        title_ = title;
        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::optional<window> {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        auto* raw = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
        if (raw == nullptr) return std::nullopt;

        return window{*raw};
    }

  private:
    int         width_{};
    int         height_{};
    std::string title_;
};

inline void poll_events() noexcept {
    glfwPollEvents();
}

inline void terminate() noexcept {
    glfwTerminate();
}
}  // namespace kgm