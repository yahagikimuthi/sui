#pragma once

#include <cstdint>
#include <utility>

#include <hgn/command.hpp>
#include <hgn/pipeline.hpp>
#include <hgn/render.hpp>
#include <hgn_raii/command.hpp>

#include "hgn_oop/instance.hpp"
#include "hgn_oop/render.hpp"

namespace hgn::oop {
class command_pool final {
  public:
    [[nodiscard]] static auto try_make(
        hgn::command_pool_create_info& info, oop::device& dev
    ) noexcept -> std::expected<oop::command_pool, hgn::result> {
        auto native = raii::try_make_command_pool(info, dev.get());
        if (not native) return std::unexpected{native.error()};

        return oop::command_pool{std::move(*native)};
    }

    [[nodiscard]] auto get() noexcept -> raii::command_pool& { return native_; }

  private:
    explicit command_pool(raii::command_pool&& native) noexcept : native_{std::move(native)} {}

    raii::command_pool native_;
};

class command_buffer final {
  public:
    [[nodiscard]] static auto try_allocate(
        const hgn::command_buffer_allocate_info& info, oop::device& dev
    ) noexcept -> std::expected<oop::command_buffer, hgn::result> {
        auto native = raii::try_allocate_command_buffer(info, dev.get());
        if (not native) return std::unexpected{native.error()};

        return oop::command_buffer{*native};
    }

    void barrier(const hgn::image_memory_barrier& info) noexcept {
        hgn::command_pipeline_barrier(native_, info);
    }

    void begin_rendering(const hgn::rendering_info& info) noexcept {
        hgn::command_begin_rendering(native_, info);
    }

    void end_rendering() noexcept { hgn::command_end_rendering(native_); }

    [[nodiscard]] auto get() noexcept -> raii::command_buffer& { return native_; }

    void bind_pipeline(oop::graphics_pipeline& pipe) noexcept {
        hgn::command_bind_pipeline(native_, pipe.get());
    }

    void set_viewport(const hgn::viewport& vp) noexcept { hgn::command_set_viewport(native_, vp); }

    void set_scissor(const hgn::rect2d& rect) noexcept { hgn::command_set_scissor(native_, rect); }

    void bind_vertex_buffer(oop::buffer& buf) noexcept {
        hgn::command_bind_vertex_buffer(native_, buf.get());
    }

    void draw(const std::uint32_t vertex_count, const std::uint32_t instance_count) noexcept {
        hgn::command_draw(native_, vertex_count, instance_count);
    }

  private:
    explicit command_buffer(const raii::command_buffer& native) noexcept : native_{native} {}

    raii::command_buffer native_;
};
}  // namespace hgn::oop