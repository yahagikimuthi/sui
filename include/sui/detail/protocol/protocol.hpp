#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <utility>

#include <hgn/buffer.hpp>
#include <hgn/command.hpp>
#include <hgn/device.hpp>
#include <hgn/image.hpp>
#include <hgn/pipeline.hpp>
#include <hgn/submit.hpp>
#include <hgn/swapchain.hpp>
#include <hgn/sync.hpp>

#include "sui/detail/debug_impl.hpp"
#include "sui/detail/hardware/device.hpp"
#include "sui/detail/others.hpp"
#include "sui/detail/protocol/command.hpp"
#include "sui/detail/protocol/sync.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class protocol final {
  public:
    [[nodiscard]] static auto try_make(
        device& dev, const std::uint32_t queue_family_indices
    ) noexcept -> std::expected<protocol, error> {
        auto sync_res = sync::try_make(dev);
        if (not sync_res)
            return make_error(runtime_error, "Failed to make sync.", sync_res.error());

        auto cmd_res = command::try_make(dev, queue_family_indices);
        if (not cmd_res)
            return make_error(runtime_error, "Failed to make command class.", cmd_res.error());

        return protocol{std::move(*sync_res), std::move(*cmd_res)};
    }

    protocol(const protocol&) noexcept                    = delete;
    auto operator=(const protocol&) noexcept -> protocol& = delete;
    protocol(protocol&&) noexcept                         = default;
    auto operator=(protocol&&) noexcept -> protocol&      = default;
    ~protocol() noexcept                                  = default;

    [[nodiscard]] auto try_wait(hgn::device& dev) noexcept -> bool {
        const auto res = sync_.try_wait_fence(dev);
        if (not res) return fail("Therefore, Failed to sync.");
        return true;
    }

    [[nodiscard]] auto acquire_next_image(hgn::device& dev, hgn::swapchain_khr& chain) noexcept
        -> std::optional<std::uint32_t> {
        auto res = hgn::try_acquire_next_image_khr(dev, chain, image_available_semaphore());
        return to_optional(res);
    }

    [[nodiscard]] auto image_available_semaphore() noexcept -> hgn::semaphore& {
        return sync_.image_available_semaphore();
    }

    [[nodiscard]] auto try_begin_rendering(hgn::image& image, hgn::image_view& view) noexcept
        -> bool {
        return command_.try_begin_rendering(image, view);
    }

    [[nodiscard]] auto try_end_rendering(hgn::image& image) noexcept -> bool {
        return command_.try_end_rendering(image);
    }

    [[nodiscard]] auto get_command_buffer() noexcept -> hgn::command_buffer& {
        return command_.buffer();
    }

    void command_bind_pipeline(hgn::pipeline& pipe) noexcept { command_.bind_pipeline(pipe); }

    void command_set_viewport(const hgn::viewport& vp) noexcept { command_.set_viewport(vp); }

    void command_set_scissor(const hgn::rect2d& rect) noexcept { command_.set_scissor(rect); }

    void command_bind_vertex_buffer(hgn::buffer& buf) noexcept { command_.bind_vertex_buffer(buf); }

    void command_draw(
        const std::uint32_t vertex_count, const std::uint32_t instance_count
    ) noexcept {
        command_.draw(vertex_count, instance_count);
    }

    [[nodiscard]] auto try_submit(hgn::queue& graphics_queue) noexcept -> bool {
        const auto info = hgn::submit_info{}
                              .setting_command_buffer(command_.buffer())
                              .wait_semaphore(sync_.image_available_semaphore())
                              .signal_semaphore(sync_.render_finished_semaphore());
        return sync_.try_submit(graphics_queue, info);
    }

    [[nodiscard]] auto make_present_info(
        hgn::swapchain_khr& chain, const std::uint32_t image_index
    ) noexcept -> hgn::present_info_khr {
        return sync_.make_present_info(chain, image_index);
    }

  private:
    explicit protocol(sync&& sync_ref, command&& cmd) noexcept
        : sync_{std::move(sync_ref)}, command_{std::move(cmd)} {}

    sync    sync_;
    command command_;
};
}  // namespace sui::detail