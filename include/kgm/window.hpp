#pragma once

#include <cassert>
#include <expected>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/instance.hpp>
#include <hgn/raw/raw.hpp>
#include <hgn/surface.hpp>

#include "kgm/detail/detail.hpp"
#include "kgm/detail/input.hpp"
#include "kgm/enums.hpp"
#include "kgm/raii/raii.hpp"
#include "kgm/raw/raw.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace kgm {
class window final {
  public:
    explicit window(raw::window& win) noexcept : raw_{std::move(win)} {
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
        static constinit auto flag = true;
        assert(flag);
        flag = false;
    }

    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept
        : terminate_{std::move(other.terminate_)},
          raw_{std::move(other.raw_)},
          input_{other.input_} {
        glfwSetWindowUserPointer(&get(), this);
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        terminate_ = std::move(other.terminate_);
        raw_       = std::move(other.raw_);
        input_     = other.input_;

        glfwSetWindowUserPointer(&get(), this);
        return *this;
    }

    ~window() noexcept = default;

    [[nodiscard]] auto is_open() noexcept -> bool { return glfwWindowShouldClose(&get()) == 0; }

    void open() noexcept { glfwSetWindowShouldClose(&get(), GLFW_FALSE); }

    void close() noexcept { glfwSetWindowShouldClose(&get(), GLFW_TRUE); }

    void poll_events() noexcept {
        glfwPollEvents();
        input_.update();
    }

    [[nodiscard]] auto key_action(const key button) noexcept -> action {
        return input_.keys.get_action(button);
    }

    [[nodiscard]] auto create_surface(hgn::instance& instance
    ) noexcept -> std::expected<hgn::surface_khr, hgn::result> {
        auto* raw = static_cast<hgn::raw::surface_khr*>(nullptr);
        return hgn::make_expected(glfwCreateWindowSurface(&instance.get(), &get(), nullptr, &raw))
            .transform([&]() noexcept -> hgn::surface_khr {
                return hgn::surface_khr{*raw, instance.get()};
            });
    }

    [[nodiscard]] auto get() noexcept -> raw::window& { return raw_; }

  private:
    detail::terminate_handler terminate_;
    raii::window              raw_;
    detail::input             input_;
};
}  // namespace kgm