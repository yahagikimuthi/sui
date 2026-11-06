#pragma once

#include <cassert>
#include <cstddef>
#include <expected>
#include <optional>
#include <span>
#include <utility>

#include <hgn/allocator.hpp>
#include <hgn/buffer.hpp>
#include <hgn/enums.hpp>
#include <hgn/hgn.hpp>
#include <hgn/physical.hpp>
#include <hgn/rendering.hpp>

#include "sui/detail/hardware.hpp"
#include "sui/detail/others.hpp"
#include "sui/error.hpp"
#include "sui/vertex.hpp"

namespace sui::detail {
class renderer final {
    static constexpr auto max_vertices    = 5000;
    static constexpr auto max_buffer_size = sizeof(vertex) * max_vertices;

  public:
    [[nodiscard]] static auto make(hardware& hard, const hgn::format fmt) noexcept
        -> std::expected<renderer, error> {
        auto allocator = hgn::get_allocator_creator()
                             .instance(hard.instance())
                             .physical(hard.physical())
                             .device(hard.device())
                             .create();
        if (not allocator) return make_error(runtime_error, "Failed to make allocator");

        auto& dev = hard.device();
        auto  vbo = allocator->get_buffer_creator()
                       .size(max_buffer_size)
                       .usage(hgn::buffer_usage_flag_bits::vertex)
                       .create();
        if (not vbo) return make_error(runtime_error, "Failed to make buffer.");

        auto ibo = allocator->get_buffer_creator()
                       .size(sizeof(std::uint32_t) * 1000)
                       .usage(hgn::buffer_usage_flag_bits::index)
                       .create();
        if (not ibo) return make_error(runtime_error, "Failed to make index buffer.");

        auto ubo = allocator->get_buffer_creator()
                       .size(sizeof(std::uint32_t) * 1000)
                       .usage(hgn::buffer_usage_flag_bits::uniform)
                       .create();

        auto vert_source = read_file("../include/sui/detail/vert.spv");
        if (not vert_source) return make_error(logic_error, "Failed to read vertex source file.");
        auto vert = dev.create_shader(*vert_source);
        if (not vert) return make_error(runtime_error, "Failed to make vertex shader module.");

        auto frag_source = read_file("../include/sui/detail/frag.spv");
        if (not frag_source) return make_error(logic_error, "Failed to read fragment source file.");
        auto frag = dev.create_shader(*frag_source);
        if (not frag) return make_error(runtime_error, "Failed to make fragment shader module.");

        auto layout = dev.create_pipeline_layout();
        if (not layout) return make_error(runtime_error, "Failed to make pipeline layout");

        auto pipe = dev.get_graphics_pipeline_creator()
                        .vertex_binding(sizeof(vertex))
                        .vertex_offsets(offsetof(vertex, position), offsetof(vertex, color))
                        .color_attachment(fmt)
                        .layout(*layout)
                        .shaders(*vert, *frag)
                        .create();

        if (not pipe) return make_error(runtime_error, "Failed to make graphics pipeline.");

        return renderer{
            std::move(*allocator),
            std::move(*vbo),
            std::move(*ibo),
            std::move(*vert),
            std::move(*frag),
            std::move(*layout),
            std::move(*pipe)
        };
    }

    void update(const std::span<const vertex> vertices) noexcept {
        assert(vertices.size() <= max_vertices);
        vbo_.cpy(vertices);
    }

    [[nodiscard]] auto vbo() noexcept -> hgn::buffer& { return vbo_; }
    [[nodiscard]] auto ibo() noexcept -> hgn::buffer& { return ibo_; }
    [[nodiscard]] auto pipeline() noexcept -> hgn::graphics_pipeline& { return pipe_; }

  private:
    explicit renderer(
        hgn::allocator&&         allocator,
        hgn::buffer&&            buffer,
        hgn::buffer&&            ibo,
        hgn::shader_module&&     vert,
        hgn::shader_module&&     frag,
        hgn::pipeline_layout&&   layout,
        hgn::graphics_pipeline&& pipe
    ) noexcept
        : allocator_{std::move(allocator)},
          vbo_{std::move(buffer)},
          ibo_{std::move(ibo)},
          vert_{std::move(vert)},
          frag_{std::move(frag)},
          layout_{std::move(layout)},
          pipe_{std::move(pipe)} {}

    hgn::allocator         allocator_;
    hgn::buffer            vbo_;
    hgn::buffer            ibo_;
    hgn::shader_module     vert_;
    hgn::shader_module     frag_;
    hgn::pipeline_layout   layout_;
    hgn::graphics_pipeline pipe_;
};
}  // namespace sui::detail