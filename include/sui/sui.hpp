#pragma once

#include <cstdint>
#include <expected>
#include <string_view>
#include <utility>

#include <akr/akr.hpp>
#include <kgm/kgm.hpp>
#include <kgm/window.hpp>

#include "sui/context.hpp"

namespace sui {
class engine final {
  public:
    [[nodiscard]] static auto create(
        const std::uint32_t width, const std::uint32_t height, const std::string_view title
    ) noexcept -> std::expected<engine, akr::error> {
        auto win = kgm::window_creator{}.extent(width, height).title(title).create();
        if (not win) return win.error().rethrow_runtime("Failed to create window.");

        auto rs = rendering_system::create(*win);
        if (not rs) return rs.error().rethrow_runtime("Failed to create rendering system.");

        return engine{std::move(*win), std::move(*rs)};
    }

    [[nodiscard]] auto get_descriptor_set_writer() noexcept;

  private:
    engine(kgm::window win, rendering_system rs) noexcept
        : win_{std::move(win)}, rs_{std::move(rs)} {}

    kgm::window      win_;
    rendering_system rs_;
};
}  // namespace sui