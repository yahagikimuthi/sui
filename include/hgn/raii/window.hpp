#pragma once

#include <optional>
#include <utility>

#include <GLFW/glfw3.h>

namespace hgn::raii {
class window final {
  public:
    explicit window(GLFWwindow&& raw) noexcept : raw_{raw} {}  // NOLINT

    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept : raw_{std::exchange(other.raw_, std::nullopt)} {}

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        return *this;
    }

    ~window() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> GLFWwindow& { return *raw_; }

    [[nodiscard]] operator GLFWwindow&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) glfwDestroyWindow(&get());
    }

    std::optional<GLFWwindow&> raw_;
};
}  // namespace hgn::raii