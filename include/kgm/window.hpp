#pragma once

#include <cassert>
#include <expected>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>
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

namespace kgm {
class window_user final {};
class window;
}  // namespace kgm

namespace kgm::detail {
class callback final {
  public:
    explicit callback() noexcept = default;

    void set(void (&fn)(window_user&, key, action)) noexcept { fn_ = &fn; }
    void reset() noexcept { fn_ = nullptr; }

    void call() noexcept;

  private:
    void (*fn_)(window_user&, key, action){};
};

class window_creator;
struct callbacks final {
    void (*keys)(window_user&, key, action){};
    void (*cursor_poss)(window_user&, float, float){};
    void (*mouse_buttons)(window_user&, mouse_button, action){};
    void (*window_sizes)(window_user&, std::uint32_t, std::uint32_t){};
};
}  // namespace kgm::detail

namespace kgm {
class window final {
    friend class detail::window_creator;

  public:
    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept : raw_{std::move(other.raw_)} {
        glfwSetWindowUserPointer(&get(), this);
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        raw_ = std::move(other.raw_);

        glfwSetWindowUserPointer(&get(), this);
        return *this;
    }

    ~window() noexcept { glfwSetWindowUserPointer(&get(), nullptr); }

    [[nodiscard]] auto should_close() noexcept -> bool {
        return glfwWindowShouldClose(&get()) != 0;
    }

    void set_user(window_user& user) noexcept { user_ = user; }

    void reset_user() noexcept { user_.reset(); }

    [[nodiscard]] auto get_user() noexcept -> std::optional<window_user&> { return user_; }

    [[nodiscard]] auto create_surface(hgn::instance& instance
    ) -> std::expected<hgn::surface_khr, hgn::result> {
        auto*      raw = static_cast<hgn::raw::surface_khr*>(nullptr);
        const auto res = glfwCreateWindowSurface(&instance.get(), &get(), nullptr, &raw);
        if (res != VK_SUCCESS) return hgn::make_unexpected(res);

        assert(raw != nullptr);
        return hgn::surface_khr{*raw, instance.get()};
    }

    void set_key_callback(void (&fn)(window_user&, key, action)) noexcept {
        callbacks_.keys = &fn;
        glfwSetKeyCallback(
            &get(),
            [](raw::window*         ptr,
               int                  button,
               [[maybe_unused]] int scancode,
               int                  act,
               [[maybe_unused]] int mods) noexcept -> void {
                auto& self = *static_cast<window*>(glfwGetWindowUserPointer(ptr));
                if (self.callbacks_.keys == nullptr) return;
                if (not self.user_) return;
                self.callbacks_.keys(
                    *self.user_, static_cast<key>(button), static_cast<action>(act)
                );
            }
        );
    }

    void set_cursor_pos_callback(void (&fn)(window_user&, float, float)) noexcept {
        callbacks_.cursor_poss = fn;
        glfwSetCursorPosCallback(
            &get(),
            [](raw::window* ptr, double xpos, double ypos) noexcept -> void {
                auto& self = *static_cast<window*>(glfwGetWindowUserPointer(ptr));
                if (self.callbacks_.cursor_poss == nullptr) return;
                if (not self.user_) return;
                self.callbacks_.cursor_poss(
                    *self.user_, static_cast<float>(xpos), static_cast<float>(ypos)
                );
            }
        );
    }

    void set_mouse_button_callback(void (&fn)(window_user&, mouse_button, action)) noexcept {
        callbacks_.mouse_buttons = &fn;
        glfwSetMouseButtonCallback(
            &get(),
            [](raw::window* ptr, int button, int act, [[maybe_unused]] int mods) noexcept -> void {
                auto& self = *static_cast<window*>(glfwGetWindowUserPointer(ptr));
                if (self.callbacks_.mouse_buttons == nullptr) return;
                if (not self.user_) return;
                self.callbacks_.mouse_buttons(
                    *self.user_, static_cast<mouse_button>(button), static_cast<action>(act)
                );
            }
        );
    }

    [[nodiscard]] auto get() noexcept -> raw::window& { return raw_; }

  private:
    explicit window(GLFWwindow& raw) noexcept : raw_{std::move(raw)} {
        glfwSetWindowUserPointer(&get(), nullptr);
    }

    raii::window               raw_;
    detail::callbacks          callbacks_;
    std::optional<window_user> user_;
};
}  // namespace kgm