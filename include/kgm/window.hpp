#pragma once

#include <expected>
#include <functional>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/instance.hpp>
#include <hgn/raw/raw.hpp>
#include <hgn/surface.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "kgm/detail/detail.hpp"
#include "kgm/enums.hpp"
#include "kgm/raii/raii.hpp"
#include "kgm/raw/raw.hpp"

namespace kgm {
class window final {
  public:
    explicit window(raw::window& win) noexcept : raw_{std::move(win)} { reset(); }

    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept
        : raw_{std::move(other.raw_)}, callbacks_{std::move(other.callbacks_)} {
        reset();
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        raw_       = std::move(other.raw_);
        callbacks_ = std::move(other.callbacks_);

        reset();
        return *this;
    }

    ~window() noexcept = default;

    [[nodiscard]] auto should_close() noexcept -> bool {
        return glfwWindowShouldClose(&get()) != 0;
    }

    auto set_key_callback(std::function<void(key, action)> fn) noexcept -> window& {
        callbacks_.keys_ = std::move(fn);

        glfwSetKeyCallback(
            &get(),
            [](raw::window*         ptr,
               int                  button,
               [[maybe_unused]] int scancode,
               int                  act,
               [[maybe_unused]] int mods) noexcept -> void {
                auto& self = *static_cast<window*>(glfwGetWindowUserPointer(ptr));
                if (self.callbacks_.keys_)
                    self.callbacks_.keys_(static_cast<key>(button), static_cast<action>(act));
            }
        );
        return *this;
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
    void reset() noexcept { glfwSetWindowUserPointer(&get(), this); }

    raii::window      raw_;
    detail::callbacks callbacks_;
};
}  // namespace kgm