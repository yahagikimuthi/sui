#pragma once

#include <expected>
#include <span>
#include <utility>

#include <hgn/window.hpp>

#include "sui/context.hpp"
#include "sui/error.hpp"
#include "sui/vertex.hpp"

namespace sui {
class window final {
  public:
    [[nodiscard]] static auto make() noexcept -> std::expected<window, error> {
        if (not hgn::window::init()) return make_error(runtime_error, "Failed to init GLFW.");

        hgn::window::set_hint(hgn::window_hint::client_api, hgn::window_value::no_api);
        auto win = hgn::window::make(800, 600, "No Title");
        if (not win) return make_error(runtime_error, "Failed to make window.");

        auto ctx = context::make(*win);
        if (not ctx) return make_error(runtime_error, "Failed to make context.", ctx.error());

        return window{std::move(*win), std::move(*ctx)};
    }

    [[nodiscard]] auto is_open() noexcept -> bool { return win_.is_open(); }

    void poll_events() noexcept { hgn::window::poll_events(); }  // NOLINT

    [[nodiscard]] auto begin_rendering() noexcept -> bool { return ctx_.begin_rendering(); }

    void render(const std::span<const vertex> vertices) noexcept { ctx_.render(vertices); }

    [[nodiscard]] auto end_rendering() noexcept -> bool { return ctx_.end_rendering(); }

  private:
    explicit window(hgn::window&& win, context&& ctx) noexcept
        : win_{std::move(win)}, ctx_{std::move(ctx)} {}

    hgn::window win_;
    context     ctx_;
};
}  // namespace sui