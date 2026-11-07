#pragma once

#include <cassert>
#include <expected>
#include <functional>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/instance.hpp>
#include <hgn/raw/raw.hpp>
#include <hgn/surface.hpp>

#include "kgm/enums.hpp"
#include "kgm/raii/window.hpp"
#include "kgm/raw/raw.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#undef GLFW_INCLUDE_VULKAN

namespace kgm::detail {
struct callbacks final {
    explicit callbacks() noexcept = default;

    std::function<void(key, action)>                  keys;
    std::function<void(float, float)>                 cursor_poss;
    std::function<void(mouse_button, action)>         mouse_buttons;
    std::function<void(std::uint32_t, std::uint32_t)> window_sizes;
};
}  // namespace kgm::detail

namespace kgm {
class window final {
  public:
    explicit window(GLFWwindow& raw) noexcept : raw_{std::move(raw)} {
        glfwSetWindowUserPointer(&get(), this);
    }

    window(const window&) noexcept                          = delete;
    auto operator=(const window&) const noexcept -> window& = delete;

    window(window&& other) noexcept
        : raw_{std::move(other.raw_)}, callbacks_{std::move(other.callbacks_)} {
        glfwSetWindowUserPointer(&get(), this);
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        raw_       = std::move(other.raw_);
        callbacks_ = std::move(other.callbacks_);

        glfwSetWindowUserPointer(&get(), this);
        return *this;
    }

    ~window() noexcept = default;

    [[nodiscard]] auto should_close() noexcept -> bool {
        return glfwWindowShouldClose(&get()) != 0;
    }

    [[nodiscard]] auto create_surface(hgn::instance& instance
    ) -> std::expected<hgn::surface_khr, hgn::result> {
        auto*      raw = static_cast<hgn::raw::surface_khr*>(nullptr);
        const auto res = glfwCreateWindowSurface(&instance.get(), &get(), nullptr, &raw);
        if (res != VK_SUCCESS) return hgn::make_unexpected(res);

        assert(raw != nullptr);
        return hgn::surface_khr{*raw, instance.get()};
    }

    void set_key_callback(std::function_ref<void(key, action)> fn) noexcept {
        callbacks_.keys = fn;
        glfwSetKeyCallback(
            &get(),
            [](raw::window*         raw,
               int                  button,
               [[maybe_unused]] int scancode,
               int                  act,
               [[maybe_unused]] int mods) noexcept -> void {
                auto& self = *static_cast<window*>(glfwGetWindowUserPointer(raw));
                if (self.callbacks_.keys)
                    self.callbacks_.keys(static_cast<key>(button), static_cast<action>(act));
            }
        );
    }

    [[nodiscard]] auto get() noexcept -> raw::window& { return raw_; }

  private:
    raii::window      raw_;
    detail::callbacks callbacks_;
};
}  // namespace kgm