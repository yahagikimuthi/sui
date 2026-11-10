#pragma once

#include <expected>
#include <optional>
#include <string_view>
#include <utility>

#include <GLFW/glfw3.h>

#include "sui/detail/camera.hpp"
#include "sui/detail/input.hpp"
#include "sui/error.hpp"
#include "sui/input.hpp"

namespace sui::detail {
class window final {
  public:
    [[nodiscard]] static auto make(
        const std::uint32_t width, const std::uint32_t height, const std::string_view title
    ) noexcept -> std::expected<window, error> {
        if (glfwInit() == 0) return make_error(runtime_error, "Failed to init GLFW.");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        auto* raw = glfwCreateWindow(
            static_cast<int>(width),
            static_cast<int>(height),
            std::string{title}.c_str(),
            nullptr,
            nullptr
        );

        if (raw == nullptr) return make_error(runtime_error, "Failed to make window.");

        return window{*raw};
    }

    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)},
          camera_{other.camera_},
          input_{other.input_} {
        reset();
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        raw_    = std::exchange(other.raw_, std::nullopt);
        camera_ = other.camera_;
        input_  = other.input_;

        reset();
        return *this;
    }

    ~window() noexcept {
        if (raw_) {
            glfwDestroyWindow(&get());
            glfwTerminate();
        }
    }

    [[nodiscard]] auto is_open() noexcept -> bool { return glfwWindowShouldClose(&get()) != 0; }

    [[nodiscard]] auto input() noexcept -> input& { return input_; }

    [[nodiscard]] auto camera() noexcept -> camera& { return camera_; }
    [[nodiscard]] auto camera() const noexcept -> const class camera& { return camera_; }

    [[nodiscard]] auto get() noexcept -> GLFWwindow& { return *raw_; }

  private:
    explicit window(GLFWwindow& raw) noexcept : raw_{raw} { reset(); }

    void reset() noexcept {
        glfwSetWindowUserPointer(&get(), this);

        glfwSetKeyCallback(
            &get(),
            [](GLFWwindow*          ptr,
               int                  button,
               [[maybe_unused]] int scancode,
               int                  act,
               [[maybe_unused]] int mods) noexcept -> void {
                auto& self = *static_cast<window*>(glfwGetWindowUserPointer(ptr));
                self.input_.keys.set(static_cast<key>(button), act == GLFW_PRESS);
            }
        );
    }

    std::optional<GLFWwindow&> raw_;
    class camera               camera_;
    struct input               input_;
};
}  // namespace sui::detail