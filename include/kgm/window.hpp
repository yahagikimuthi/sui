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

#include <GLFW/glfw3.h>

namespace kgm {
template <typename User>
    requires std::is_class_v<User>
class window;
}  // namespace kgm

namespace kgm::detail {
template <typename User>
struct callbacks final {
    explicit callbacks() noexcept = default;

    void (*keys)(window<User>&, key, action){};
    void (*cursor_poss)(window<User>&, float, float){};
    void (*mouse_buttons)(window<User>&, mouse_button, action){};
    void (*window_sizes)(window<User>&, std::uint32_t, std::uint32_t){};
};
}  // namespace kgm::detail

namespace kgm {
template <typename User>
    requires std::is_class_v<User>
class window final {
  public:
    explicit window(GLFWwindow& raw) noexcept : raw_{std::move(raw)} {
        glfwSetWindowUserPointer(&get(), this);
    }

    window(const window&) noexcept                          = delete;
    auto operator=(const window&) const noexcept -> window& = delete;

    window(window&& other) noexcept
        : raw_{std::move(other.raw_)}, callbacks_{other.callbacks_}, user_{other.user_} {
        glfwSetWindowUserPointer(&get(), this);
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        raw_       = std::move(other.raw_);
        callbacks_ = other.callbacks_;
        user_      = other.user_;

        glfwSetWindowUserPointer(&get(), this);
        return *this;
    }

    ~window() noexcept = default;

    [[nodiscard]] auto should_close() noexcept -> bool {
        return glfwWindowShouldClose(&get()) != 0;
    }

    void set_user(User& user) noexcept { user_ = user; }

    void reset_user() noexcept { user_ = nullptr; }

    [[nodiscard]] auto get_user() noexcept -> std::optional<User&> { return user_; }

    [[nodiscard]] auto create_surface(hgn::instance& instance
    ) -> std::expected<hgn::surface_khr, hgn::result> {
        auto*      raw = static_cast<hgn::raw::surface_khr*>(nullptr);
        const auto res = glfwCreateWindowSurface(&instance.get(), &get(), nullptr, &raw);
        if (res != VK_SUCCESS) return hgn::make_unexpected(res);

        assert(raw != nullptr);
        return hgn::surface_khr{*raw, instance.get()};
    }

    [[nodiscard]] auto get() noexcept -> raw::window& { return raw_; }

  private:
    raii::window            raw_;
    detail::callbacks<User> callbacks_;
    std::optional<User&>    user_;
};
}  // namespace kgm