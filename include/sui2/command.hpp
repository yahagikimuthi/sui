#pragma once

#include <expected>
#include <utility>

#include <hgn2/command.hpp>

#include "sui2/error.hpp"
#include "sui2/hardware.hpp"

namespace sui {
class command final {
  public:
    [[nodiscard]] static auto make(hardware& hard) noexcept -> std::expected<command, error> {
        auto& dev  = hard.device();
        auto  pool = dev.make_command_pool(hard.graphics_queue().queue_family_index());
        if (not pool) return make_error(runtime_error, "Failed to make command pool.");

        auto buffer = dev.allocate_command_buffer(*pool);
        if (not buffer) return make_error(runtime_error, "Failed to allocate command buffer.");

        return command{std::move(*pool), std::move(*buffer)};
    }

  private:
    explicit command(hgn::command_pool&& pool, hgn::command_buffer&& buffer) noexcept
        : pool_{std::move(pool)}, buffer_{std::move(buffer)} {}

    hgn::command_pool   pool_;
    hgn::command_buffer buffer_;
};
}  // namespace sui