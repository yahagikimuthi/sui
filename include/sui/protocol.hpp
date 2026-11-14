#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>

#include "sui/command.hpp"
#include "sui/hardware.hpp"
#include "sui/sync.hpp"

namespace sui {
class protocol final {
  public:
    [[nodiscard]] static auto create(hardware& hard
    ) noexcept -> std::expected<protocol, akr::error> {
        auto sync_obj = sync::create(hard.device());
        if (not sync_obj)
            return akr::make_error(
                akr::runtime_error, "Failed to create sync objects.", sync_obj.error()
            );

        auto cmd = command_buffer::create(hard);
        if (not cmd)
            return akr::make_error(
                akr::runtime_error, "Failed to create command buffer.", cmd.error()
            );

        return protocol{std::move(*sync_obj), std::move(*cmd)};
    }

  private:
    protocol(sync sync, command_buffer cmd) noexcept
        : sync_{std::move(sync)}, cmd_{std::move(cmd)} {}

    sync           sync_;
    command_buffer cmd_;
};
}  // namespace sui