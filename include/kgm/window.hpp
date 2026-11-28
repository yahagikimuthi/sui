#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <memory>
#include <utility>

#include <glm/glm.hpp>
#include <hgn/enums.hpp>
#include <hgn/instance.hpp>
#include <hgn/raw/raw.hpp>
#include <hgn/surface.hpp>

#include "kgm/detail/detail.hpp"
#include "kgm/detail/input.hpp"
#include "kgm/enums.hpp"
#include "kgm/raw/raw.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace kgm {
class window final {
  public:
    explicit window(
        raw::window& win, const std::uint32_t width, const std::uint32_t height
    ) noexcept
        : raw_{&win, glfwDestroyWindow}, width_{width}, height_{height} {
        glfwSetWindowUserPointer(&get(), this);

        glfwSetKeyCallback(
            &get(),
            [](raw::window*         ptr,
               int                  button,
               [[maybe_unused]] int scancode,
               int                  act,
               [[maybe_unused]] int mods) noexcept -> void {
                auto& self = *static_cast<window*>(glfwGetWindowUserPointer(ptr));
                self.input_.keys.set(static_cast<key>(button), act);
            }
        );

        glfwSetWindowSizeCallback(&get(), [](raw::window* ptr, int w, int h) noexcept -> void {
            auto& self   = *static_cast<window*>(glfwGetWindowUserPointer(ptr));
            self.width_  = static_cast<std::uint32_t>(w);
            self.height_ = static_cast<std::uint32_t>(h);
        });

        glfwSetCursorPosCallback(&get(), [](raw::window* ptr, double x, double y) noexcept -> void {
            auto& self          = *static_cast<window*>(glfwGetWindowUserPointer(ptr));
            self.input_.cursor_ = {x, y};
        });

        auto cursor_x = 0.0;
        auto cursor_y = 0.0;
        glfwGetCursorPos(&get(), &cursor_x, &cursor_y);
        input_.cursor_ = {cursor_x, cursor_y};

        static constinit auto flag = true;
        assert(flag);
        flag = false;
    }

    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept
        : terminate_{std::move(other.terminate_)},
          raw_{std::move(other.raw_)},
          input_{other.input_},
          width_{other.width_},
          height_{other.height_} {
        glfwSetWindowUserPointer(&get(), this);
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        terminate_ = std::move(other.terminate_);
        raw_       = std::move(other.raw_);
        input_     = other.input_;
        width_     = other.width_;
        height_    = other.height_;

        glfwSetWindowUserPointer(&get(), this);
        return *this;
    }

    ~window() noexcept = default;

    void open() noexcept { glfwSetWindowShouldClose(&get(), GLFW_FALSE); }

    void close() noexcept { glfwSetWindowShouldClose(&get(), GLFW_TRUE); }

    void poll_events() noexcept {
        glfwPollEvents();
        input_.update();
    }

    [[nodiscard]] auto key_action(const key button) const noexcept -> action {
        return input_.keys.get_action(button);
    }

    [[nodiscard]] auto create_surface(hgn::instance& instance
    ) noexcept -> std::expected<hgn::surface_khr, hgn::result> {
        auto* raw = static_cast<hgn::raw::surface_khr*>(nullptr);
        return hgn::to_expected(glfwCreateWindowSurface(&instance.get(), &get(), nullptr, &raw))
            .transform([&]() noexcept -> hgn::surface_khr {
                return hgn::surface_khr{*raw, instance.get()};
            });
    }

    [[nodiscard]] auto is_open() noexcept -> bool { return glfwWindowShouldClose(&get()) == 0; }

    [[nodiscard]] auto width() const noexcept -> std::uint32_t { return width_; }

    [[nodiscard]] auto height() const noexcept -> std::uint32_t { return height_; }

    [[nodiscard]] auto cursor() const noexcept -> glm::vec2 { return input_.cursor_; }

    [[nodiscard]] auto cursor_input_mode() noexcept -> enum cursor {
        return static_cast<enum cursor>(glfwGetInputMode(&get(), GLFW_CURSOR));
    };

    void cursor_input_mode(const enum cursor mode) noexcept {
        glfwSetInputMode(&get(), GLFW_CURSOR, static_cast<int>(mode));
    }

    [[nodiscard]] auto get() noexcept -> raw::window& { return *raw_; }

  private:
    detail::terminate_handler                            terminate_;
    std::unique_ptr<raw::window, void (*)(raw::window*)> raw_;
    detail::input                                        input_;
    std::uint32_t                                        width_;
    std::uint32_t                                        height_;
};
}  // namespace kgm