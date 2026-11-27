#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/command.hpp>

#include "wk/detail/hardware.hpp"

namespace wk::detail {
class command final {
  public:
    [[nodiscard]] static auto create(hardware& hard
    ) noexcept -> std::expected<command, akr::error> {
        auto& dev  = hard.device();
        auto  pool = dev.create_command_pool(hard.graphics_queue().family());
        if (not pool) return akr::throw_runtime("Failed to create command pool.");

        auto buffer = pool->create_buffer();
        if (not buffer) return akr::throw_runtime("Failed to create command buffer.");

        return command{std::move(*pool), *buffer};
    }

    [[nodiscard]] auto pool() noexcept -> hgn::command_pool& { return pool_; }

    [[nodiscard]] auto buffer() noexcept -> hgn::command_buffer& { return buffer_; }

  private:
    command(hgn::command_pool pool, hgn::command_buffer buffer) noexcept
        : pool_{std::move(pool)}, buffer_{buffer} {}

    hgn::command_pool   pool_;
    hgn::command_buffer buffer_;
};
}  // namespace wk::detail