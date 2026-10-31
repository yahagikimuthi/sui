#pragma once

#include <expected>
#include <utility>

#include <hgn/command.hpp>

#include "sui/error.hpp"
#include "sui/hardware.hpp"

namespace sui {
class command final {
  public:
    [[nodiscard]] static auto make(hardware& hard) noexcept -> std::expected<command, error> {
        auto& dev  = hard.device();
        auto  pool = dev.make_command_pool(hard.graphics_queue().queue_family_index());
        if (not pool) return make_error(runtime_error, "Failed to make command pool.");

        auto buffer = pool->allocate_buffer();
        if (not buffer) return make_error(runtime_error, "Failed to allocate command buffer.");

        return command{std::move(*pool), std::move(*buffer)};
    }

    [[nodiscard]] auto buffer() noexcept -> hgn::command_buffer& { return buffer_; }

  private:
    explicit command(hgn::command_pool&& pool, hgn::command_buffer&& buffer) noexcept
        : pool_{std::move(pool)}, buffer_{std::move(buffer)} {}

    hgn::command_pool   pool_;
    hgn::command_buffer buffer_;
};
}  // namespace sui