#pragma once

#include <cstdint>
#include <expected>
#include <utility>

#include <hgn/command.hpp>
#include <hgn/device.hpp>
#include <hgn/image.hpp>
#include <hgn/submit.hpp>
#include <hgn/sync.hpp>

#include "sui/detail/protocol/command.hpp"
#include "sui/detail/protocol/sync.hpp"
#include "sui/error.hpp"

namespace sui {
class protocol final {
  public:
    [[nodiscard]] static auto try_make(
        hgn::device& dev, const std::uint32_t queue_family_indices
    ) noexcept -> std::expected<protocol, error> {
        auto sync_res = sync::try_make(dev);
        if (not sync_res)
            return make_error(runtime_error, "Failed to make sync.", sync_res.error());

        auto cmd_res = command::try_make(dev, queue_family_indices);
        if (not cmd_res)
            return make_error(runtime_error, "Failed to make command class.", cmd_res.error());

        return protocol{std::move(*sync_res), std::move(*cmd_res)};
    }

    [[nodiscard]] auto try_wait(hgn::device& dev) noexcept -> bool {
        const auto res = sync_.try_wait_fence(dev);
        if (not res) {
            debugger::notify("Therefore, Failed to sync.");
            return false;
        }
        return true;
    }

    [[nodiscard]] auto image_available_semaphore() noexcept -> hgn::semaphore& {
        return sync_.image_available_semaphore();
    }

    [[nodiscard]] auto render_finished_semaphore() noexcept -> hgn::semaphore& {
        return sync_.render_finished_semaphore();
    }

    [[nodiscard]] auto in_flight_fence() noexcept -> hgn::fence& { return sync_.in_flight_fence(); }

    [[nodiscard]] auto try_begin_rendering(hgn::image& image, hgn::image_view& view) noexcept
        -> bool {
        return command_.try_begin_rendering(image, view);
    }

    [[nodiscard]] auto try_end_rendering(hgn::image& image) noexcept -> bool {
        return command_.try_end_rendering(image);
    }

    [[nodiscard]] auto make_submit_info() noexcept -> hgn::submit_info {
        auto info = hgn::submit_info{};
        info.setting_command_buffer(command_.buffer())
            .wait_semaphore(sync_.image_available_semaphore())
            .signal_semaphore(sync_.render_finished_semaphore());
        return info;
    }

    [[nodiscard]] auto get_command_buffer() noexcept -> hgn::command_buffer& {
        return command_.buffer();
    }

  private:
    explicit protocol(sync&& sync_ref, command&& cmd) noexcept
        : sync_{std::move(sync_ref)}, command_{std::move(cmd)} {}

    sync    sync_;
    command command_;
};
}  // namespace sui