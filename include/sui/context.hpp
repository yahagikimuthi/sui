#pragma once

#include <cstdint>
#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/allocator.hpp>
#include <hgn/hgn.hpp>
#include <kgm/window.hpp>

#include "sui/command.hpp"
#include "sui/display.hpp"
#include "sui/hardware.hpp"
#include "sui/sync.hpp"

namespace sui {
class context final {
  public:
    [[nodiscard]] static auto create(kgm::window& win
    ) noexcept -> std::expected<context, akr::error> {
        auto hard = hardware::create(win);
        if (not hard)
            return akr::make_error(
                akr::runtime_error, "Failed to create hardware object.", hard.error()
            );

        auto display = display::create(*hard);
        if (not display)
            return akr::make_error(
                akr::runtime_error, "Failed to create display object.", display.error()
            );

        auto sync = sync::create(hard->device());
        if (not sync) return akr::make_error(akr::runtime_error, "Failed to create sync objects.");

        auto cmd = command_buffer::create(*hard);

        if (not cmd)
            return akr::make_error(akr::runtime_error, "Failed to create command objects.");

        auto allocator =
            hgn::allocator_creator{}.instance(hard->instance()).device(hard->device()).create();
        if (not allocator)
            return akr::make_error(akr::runtime_error, "Failed to create allocator.");

        return context{
            std::move(*hard),
            std::move(*display),
            std::move(*sync),
            std::move(*cmd),
            std::move(*allocator)
        };
    }

  private:
    context(
        hardware hard, display display, sync sync, command_buffer cmd, hgn::allocator allocator
    ) noexcept
        : hard_{std::move(hard)},
          display_{std::move(display)},
          sync_{std::move(sync)},
          cmd_{std::move(cmd)},
          allocator_{std::move(allocator)} {}

    hardware       hard_;
    display        display_;
    sync           sync_;
    command_buffer cmd_;
    hgn::allocator allocator_;
    std::uint32_t  image_index_{};
};
}  // namespace sui