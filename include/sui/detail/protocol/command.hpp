#pragma once

#include <cstdint>
#include <utility>

#include <hgn/buffer.hpp>
#include <hgn/command.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <hgn/pipeline.hpp>
#include <hgn/render.hpp>
#include <hgn_raii/command.hpp>
#include <hgn_raii/device.hpp>

#include "sui/error.hpp"

namespace sui::detail {
class command final {
  public:
    [[nodiscard]] static auto try_make(
        hgn::raii::device& dev, const std::uint32_t queue_family_indices
    ) noexcept -> std::expected<command, error> {
        const auto info = hgn::command_pool_create_info{}.queue_family_index(queue_family_indices);
        auto       pool = hgn::raii::try_make_command_pool(info, dev.get());

        if (not pool) return make_error(runtime_error, "Failed to make command pool.");

        const auto alloc_info = hgn::command_buffer_allocate_info{}.command_pool(pool->get());
        auto       buffer     = hgn::raii::try_allocate_command_buffer(alloc_info, dev.get());
        if (not buffer) return make_error(runtime_error, "Failed to make command buffer.");

        return command{std::move(*pool), *buffer};
    }

    command(const command&) noexcept                    = delete;
    auto operator=(const command&) noexcept -> command& = delete;
    command(command&&) noexcept                         = default;
    auto operator=(command&&) noexcept -> command&      = default;
    ~command() noexcept                                 = default;

    [[nodiscard]] auto try_begin_rendering(hgn::image& image, hgn::image_view& view) noexcept
        -> bool {
        const auto res = hgn::try_begin_command_buffer(buffer_, hgn::command_buffer_begin_info{});
        if (not res) return false;

        const auto barrier =
            hgn::image_memory_barrier{}
                .layout(hgn::image_layout::undefined, hgn::image_layout::color_attachment_optimal)
                .setting_image(image);

        hgn::command_pipeline_barrier(buffer_, barrier);

        const auto color_attachment =
            hgn::rendering_attachment_info{}.setting_image_view(view).clear_color(
                0.1f, 0.2f, 0.4f, 1.f
            );

        const auto render_info =
            hgn::rendering_info{}.extent(800, 600).color_attachment(color_attachment);

        hgn::command_begin_rendering(buffer_, render_info);
        return true;
    }

    [[nodiscard]] auto try_end_rendering(hgn::image& image) noexcept -> bool {
        hgn::command_end_rendering(buffer_);

        auto barrier = hgn::image_memory_barrier{};
        barrier
            .layout(hgn::image_layout::color_attachment_optimal, hgn::image_layout::present_src_khr)
            .setting_image(image);

        hgn::command_pipeline_barrier(buffer_, barrier);

        auto res = hgn::try_end_command_buffer(buffer_);
        return res.has_value();
    }

    [[nodiscard]] auto buffer() noexcept -> hgn::command_buffer& { return buffer_.get(); }

    void bind_pipeline(hgn::pipeline& pipe) noexcept { hgn::command_bind_pipeline(buffer_, pipe); }

    void set_viewport(const hgn::viewport& vp) noexcept { hgn::command_set_viewport(buffer_, vp); }

    void set_scissor(const hgn::rect2d& rect) noexcept { hgn::command_set_scissor(buffer_, rect); }

    void bind_vertex_buffer(hgn::buffer& buf) noexcept {
        hgn::command_bind_vertex_buffer(buffer_, buf);
    }

    void draw(const std::uint32_t vertex_count, const std::uint32_t instance_count) noexcept {
        hgn::command_draw(buffer_, vertex_count, instance_count);
    }

  private:
    explicit command(
        hgn::raii::command_pool&& pool, const hgn::raii::command_buffer& buffer
    ) noexcept
        : pool_{std::move(pool)}, buffer_{buffer} {}

    hgn::raii::command_pool   pool_;
    hgn::raii::command_buffer buffer_;
};
}  // namespace sui::detail