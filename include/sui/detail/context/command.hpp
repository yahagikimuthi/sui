#pragma once

#include <expected>
#include <utility>

#include <hgn/command.hpp>

#include "sui/detail/context/hardware.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class command final {
  public:
    [[nodiscard]] static auto make(hardware& hard) noexcept -> std::expected<command, error> {
        auto& dev  = hard.device();
        auto  pool = dev.create_command_pool(hard.graphics_family());
        if (not pool) return make_error(runtime_error, "Failed to make command pool.");

        auto buffer = pool->create_buffer();
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
}  // namespace sui::detail