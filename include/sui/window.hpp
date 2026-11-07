#pragma once

#include <expected>
#include <span>
#include <utility>

#include <kgm/kgm.hpp>
#include <kgm/window.hpp>

#include "sui/detail/context.hpp"
#include "sui/detail/window.hpp"
#include "sui/error.hpp"
#include "sui/vertex.hpp"

namespace sui {
class window final {
  public:
    [[nodiscard]] static auto make() noexcept -> std::expected<window, error> {
        auto win = detail::window::make(800, 600, "No Title");

        if (not win) return make_error(runtime_error, "Failed to make window.");

        auto ctx = detail::context::make(win->get());
        if (not ctx) return make_error(runtime_error, "Failed to make context.", ctx.error());

        return window{std::move(*win), std::move(*ctx)};
    }

    [[nodiscard]] auto is_open() noexcept -> bool { return win_.is_open(); }

    void poll_events() noexcept { kgm::poll_events(); }  // NOLINT

    [[nodiscard]] auto begin_rendering() noexcept -> bool { return ctx_.begin_rendering(); }

    void render(const std::span<const vertex> vertices) noexcept { ctx_.render(vertices); }

    void render(
        const std::span<const vertex> vertices, const std::span<const std::uint32_t> indices
    ) noexcept {
        ctx_.render(vertices, indices);
    }

    [[nodiscard]] auto end_rendering() noexcept -> bool { return ctx_.end_rendering(); }

  private:
    explicit window(detail::window&& win, detail::context&& ctx) noexcept
        : win_{std::move(win)}, ctx_{std::move(ctx)} {}

    detail::window  win_;
    detail::context ctx_;
};
}  // namespace sui