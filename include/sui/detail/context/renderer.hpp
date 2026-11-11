#pragma once

#include <cstddef>
#include <expected>
#include <optional>
#include <span>
#include <utility>

#include <hgn/allocator.hpp>
#include <hgn/buffer.hpp>
#include <hgn/descriptor.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/hgn.hpp>
#include <hgn/physical.hpp>
#include <hgn/rendering.hpp>

#include "hgn/image.hpp"
#include "sui/detail/context/hardware.hpp"
#include "sui/detail/others.hpp"
#include "sui/error.hpp"
#include "sui/vertex.hpp"

namespace sui::detail {
class buffer final {
    static constexpr auto max_vertices    = 5000;
    static constexpr auto max_buffer_size = sizeof(vertex) * max_vertices;

  public:
    [[nodiscard]] static auto make(hardware& hard) noexcept -> std::expected<buffer, error> {
        auto allocator = hgn::get_allocator_creator()
                             .instance(hard.instance())
                             .physical(hard.physical())
                             .device(hard.device())
                             .create();
        if (not allocator) return make_error(runtime_error, "Failed to make allocator");

        auto vertex = allocator->get_buffer_creator()
                          .size(max_buffer_size)
                          .usage(hgn::buffer_usage_flag_bits::vertex)
                          .create();
        if (not vertex) return make_error(runtime_error, "Failed to make vertex buffer.");

        auto index = allocator->get_buffer_creator()
                         .size(max_buffer_size)
                         .usage(hgn::buffer_usage_flag_bits::index)
                         .create();
        if (not index) return make_error(runtime_error, "Failed to make index buffer.");

        auto uniform = allocator->get_buffer_creator()
                           .size(max_buffer_size)
                           .usage(hgn::buffer_usage_flag_bits::uniform)
                           .create();
        if (not uniform) return make_error(runtime_error, "Failed to make uniform buffer.");

        return buffer{
            std::move(*allocator), std::move(*vertex), std::move(*index), std::move(*uniform)
        };
    }

    [[nodiscard]] auto vert() noexcept -> hgn::buffer& { return vertex_; }

    [[nodiscard]] auto index() noexcept -> hgn::buffer& { return index_; }

    [[nodiscard]] auto uniform() noexcept -> hgn::buffer& { return uniform_; }

  private:
    explicit buffer(
        hgn::allocator&& allocator, hgn::buffer&& vertex, hgn::buffer&& index, hgn::buffer&& uniform
    ) noexcept
        : allocator_{std::move(allocator)},
          vertex_{std::move(vertex)},
          index_{std::move(index)},
          uniform_{std::move(uniform)} {}

    hgn::allocator allocator_;
    hgn::buffer    vertex_;
    hgn::buffer    index_;
    hgn::buffer    uniform_;
};

class shader final {
  public:
    [[nodiscard]] static auto make(hgn::device& dev) noexcept -> std::expected<shader, error> {
        auto vert_source = read_file("../include/sui/detail/shader/vert.spv");
        if (not vert_source) return make_error(logic_error, "Failed to read vertex source file.");
        auto vert = dev.create_shader(*vert_source);
        if (not vert) return make_error(runtime_error, "Failed to make vertex shader module.");

        auto frag_source = read_file("../include/sui/detail/shader/frag.spv");
        if (not frag_source) return make_error(logic_error, "Failed to read fragment source file.");
        auto frag = dev.create_shader(*frag_source);
        if (not frag) return make_error(runtime_error, "Failed to make fragment shader module.");

        return shader{std::move(*vert), std::move(*frag)};
    }

    [[nodiscard]] auto vertex() noexcept -> hgn::shader_module& { return vert_; }
    [[nodiscard]] auto fragment() noexcept -> hgn::shader_module& { return frag_; }

  private:
    explicit shader(hgn::shader_module&& vert, hgn::shader_module&& frag) noexcept
        : vert_{std::move(vert)}, frag_{std::move(frag)} {}

    hgn::shader_module vert_;
    hgn::shader_module frag_;
};

class descriptor final {
  public:
    [[nodiscard]] static auto make(hgn::device& dev, hgn::buffer& uniform) noexcept
        -> std::expected<descriptor, error> {
        auto layout = dev.create_descriptor_set_layout();
        if (not layout) return make_error(runtime_error, "Failed to make descriptor set layout.");

        auto pool = dev.create_descriptor_pool();
        if (not pool) return make_error(runtime_error, "Failed to make descriptor pool.");

        auto set = pool->get_descriptor_set(*layout);
        if (not set) return make_error(runtime_error, "Failed to make descriptor set.");

        set->update(uniform);

        return descriptor{std::move(*layout), std::move(*pool), *set};
    }

    [[nodiscard]] auto layout() noexcept -> hgn::descriptor_set_layout& { return layout_; }

    [[nodiscard]] auto set() noexcept -> hgn::descriptor_set& { return set_; }

  private:
    explicit descriptor(
        hgn::descriptor_set_layout&& layout, hgn::descriptor_pool&& pool, hgn::descriptor_set set
    ) noexcept
        : layout_{std::move(layout)}, pool_{std::move(pool)}, set_{set} {}

    hgn::descriptor_set_layout layout_;
    hgn::descriptor_pool       pool_;
    hgn::descriptor_set        set_;
};

class renderer final {
  public:
    [[nodiscard]] static auto make(hardware& hard, const hgn::format fmt) noexcept
        -> std::expected<renderer, error> {
        auto buffer = buffer::make(hard);
        if (not buffer) return make_error(runtime_error, "Failed to make buffer.", buffer.error());

        auto& dev = hard.device();

        auto shader = shader::make(dev);
        if (not shader) return make_error(runtime_error, "Failed to make shader", shader.error());

        auto descriptor = descriptor::make(dev, buffer->uniform());
        if (not descriptor)
            return make_error(runtime_error, "Failed to make descriptor", descriptor.error());

        auto layout = dev.create_pipeline_layout(descriptor->layout());
        if (not layout) return make_error(runtime_error, "Failed to make pipeline layout");

        auto pipe = dev.get_graphics_pipeline_creator()
                        .vertex_binding(sizeof(vertex))
                        .vertex_offsets(offsetof(vertex, position), offsetof(vertex, color))
                        .color_attachment(fmt)
                        .layout(*layout)
                        .shaders(shader->vertex(), shader->fragment())
                        .create();

        if (not pipe) return make_error(runtime_error, "Failed to make graphics pipeline.");

        auto sampler = dev.create_sampler();
        if (not sampler) return make_error(runtime_error, "Failed to make sampler.");

        return renderer{
            std::move(*buffer),
            std::move(*shader),
            std::move(*layout),
            std::move(*pipe),
            std::move(*descriptor),
            std::move(*sampler)
        };
    }

    [[nodiscard]] auto vbo() noexcept -> hgn::buffer& { return buffer_.vert(); }
    [[nodiscard]] auto ibo() noexcept -> hgn::buffer& { return buffer_.index(); }
    [[nodiscard]] auto ubo() noexcept -> hgn::buffer& { return buffer_.uniform(); }
    [[nodiscard]] auto pipeline_layout() noexcept -> hgn::pipeline_layout& { return layout_; }
    [[nodiscard]] auto pipeline() noexcept -> hgn::graphics_pipeline& { return pipe_; }
    [[nodiscard]] auto descriptor_set() noexcept -> hgn::descriptor_set& {
        return descriptor_.set();
    }

  private:
    explicit renderer(
        buffer&&                 buffer,
        shader&&                 shader,
        hgn::pipeline_layout&&   layout,
        hgn::graphics_pipeline&& pipe,
        descriptor&&             descriptor,
        hgn::sampler&&           sampler
    ) noexcept
        : buffer_{std::move(buffer)},
          shader_{std::move(shader)},
          descriptor_{std::move(descriptor)},
          layout_{std::move(layout)},
          pipe_{std::move(pipe)},
          sampler_{std::move(sampler)} {}

    buffer                 buffer_;
    shader                 shader_;
    descriptor             descriptor_;
    hgn::pipeline_layout   layout_;
    hgn::graphics_pipeline pipe_;
    hgn::sampler           sampler_;
};
}  // namespace sui::detail