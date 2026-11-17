#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/command.hpp>

#include "sui/hardware.hpp"

namespace sui {
class command final {
  public:
    [[nodiscard]] static auto create(hardware& hard
    ) noexcept -> std::expected<command, akr::error> {
        auto& dev  = hard.device();
        auto  pool = dev.create_command_pool(hard.graphics_queue().family());
        if (not pool) return akr::throw_runtime("Failed to create command pool.");

        auto buffer = pool->create_buffer();
        if (not buffer) return akr::throw_runtime("Failed to create command buffer.");

        return command{std::move(*pool), std::move(*buffer)};
    }

    [[nodiscard]] auto buffer() noexcept -> hgn::command_buffer& { return buffer_; }

  private:
    command(hgn::command_pool pool, hgn::command_buffer buffer) noexcept
        : pool_{std::move(pool)}, buffer_{std::move(buffer)} {}

    hgn::command_pool   pool_;
    hgn::command_buffer buffer_;
};
}  // namespace sui