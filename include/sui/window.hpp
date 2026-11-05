#pragma once

#include <expected>
#include <span>
#include <utility>

#include <kgm/kgm.hpp>
#include <kgm/window.hpp>

#include "kgm/enums.hpp"
#include "sui/detail/context.hpp"
#include "sui/error.hpp"
#include "sui/vertex.hpp"

namespace sui {
class window final {
  public:
    [[nodiscard]] static auto make() noexcept -> std::expected<window, error> {
        if (not kgm::init()) return make_error(runtime_error, "Failed to init GLFW.");

        auto win = kgm::get_window_creator()
                       .hint(kgm::window_hint::client_api, kgm::window_value::no_api)
                       .extent(800, 600)
                       .title("NoTitle")
                       .create();

        if (not win) return make_error(runtime_error, "Failed to make window.");

        auto ctx = detail::context::make(*win);
        if (not ctx) return make_error(runtime_error, "Failed to make context.", ctx.error());

        return window{std::move(*win), std::move(*ctx)};
    }

    [[nodiscard]] auto is_open() noexcept -> bool { return not win_.should_close(); }

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
    explicit window(kgm::window&& win, detail::context&& ctx) noexcept
        : win_{std::move(win)}, ctx_{std::move(ctx)} {}

    kgm::window     win_;
    detail::context ctx_;
};
}  // namespace sui