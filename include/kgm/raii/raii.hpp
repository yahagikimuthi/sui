#pragma once

#include <optional>
#include <utility>

#include <GLFW/glfw3.h>

#include "kgm/raw/raw.hpp"

namespace kgm::raii {
class window final {
  public:
    explicit window(raw::window&& raw) noexcept : raw_{raw} {}  // NOLINT

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

    [[nodiscard]] auto get() noexcept -> raw::window& { return *raw_; }

    [[nodiscard]] operator raw::window&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) glfwDestroyWindow(&get());
    }

    std::optional<raw::window&> raw_;
};
}  // namespace kgm::raii