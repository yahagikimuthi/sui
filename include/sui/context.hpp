#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <kgm/window.hpp>

#include "sui/display.hpp"
#include "sui/hardware.hpp"
#include "sui/protocol.hpp"

namespace sui {
class context final {
  public:
    [[nodiscard]] static auto create(kgm::window& win
    ) noexcept -> std::expected<context, akr::error> {
        auto hard = hardware::create(win);
        if (not hard)
            return akr::make_error(
                akr::runtime_error, "Failed to create hardware class.", hard.error()
            );

        auto display = display::create(*hard);
        if (not display)
            return akr::make_error(
                akr::runtime_error, "Failed to create display class.", display.error()
            );

        auto protocol = protocol::create(*hard);
        if (not protocol)
            return akr::make_error(
                akr::runtime_error, "Failed to create protocol class.", protocol.error()
            );

        return context{std::move(*hard), std::move(*display), std::move(*protocol)};
    }

  private:
    context(hardware hard, display display, protocol protocol) noexcept
        : hard_{std::move(hard)}, display_{std::move(display)}, protocol_{std::move(protocol)} {}

    hardware hard_;
    display  display_;
    protocol protocol_;
};
}  // namespace sui