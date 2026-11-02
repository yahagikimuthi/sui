#pragma once

#include <cassert>
#include <expected>
#include <optional>
#include <type_traits>
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

namespace kgm::detail {
class window_creator;
}  // namespace kgm::detail

class window_user final {};

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

    static void set_user(window_user& user) noexcept { user_ = user; }

    static void reset_user() noexcept { user_.reset(); }

    [[nodiscard]] static auto get_user() noexcept -> std::optional<window_user&> { return user_; }

    [[nodiscard]] auto create_surface(hgn::instance& instance)
        -> std::expected<hgn::surface_khr, hgn::result> {
        auto*      raw = static_cast<hgn::raw::surface_khr*>(nullptr);
        const auto res = glfwCreateWindowSurface(&instance.get(), &get(), nullptr, &raw);
        if (res != VK_SUCCESS) return hgn::make_unexpected(res);

        assert(raw != nullptr);
        return hgn::surface_khr{*raw, instance.get()};
    }

    template <detail::callbackable<key, action> F>
    void set_key_callback(F&& callback) noexcept {
        static thread_local auto fn             = F{};
        fn                                      = std::forward<F>(callback);
        static thread_local constinit auto* raw = static_cast<raw::window*>(nullptr);
        raw                                     = &get();

        glfwSetKeyCallback(
            &get(),
            [](raw::window*         ptr,
               int                  button,
               [[maybe_unused]] int scancode,
               int                  act,
               [[maybe_unused]] int mods) noexcept -> void {
                auto* self = static_cast<window*>(glfwGetWindowUserPointer(ptr));
                fn(self)
            }
        );
    }

    [[nodiscard]] auto get() noexcept -> raw::window& { return raw_; }

  private:
    explicit window(GLFWwindow& raw) noexcept : raw_{std::move(raw)} {
        glfwSetWindowUserPointer(&get(), nullptr);
    }

    raii::window                       raw_;
    static std::optional<window_user&> user_;
};
}  // namespace kgm