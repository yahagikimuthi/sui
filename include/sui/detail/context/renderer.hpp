#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <utility>

#include <hgn/allocator.hpp>
#include <hgn/buffer.hpp>
#include <hgn/descriptor.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/hgn.hpp>
#include <hgn/image.hpp>
#include <hgn/physical.hpp>
#include <hgn/rendering.hpp>
#include <sbwi/sbwi.hpp>

#include "hgn/command.hpp"
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

    [[nodiscard]] auto allocator() noexcept -> hgn::allocator& { return allocator_; }

    [[nodiscard]] auto vert() noexcept -> hgn::buffer& { return vertex_; }

    [[nodiscard]] auto index() noexcept -> hgn::buffer& { return index_; }

    [[nodiscard]] auto uniform() noexcept -> hgn::buffer& { return uniform_; }

  private:
    buffer(
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
    [[nodiscard]] static auto make(
        hgn::device&                 dev,
        const std::filesystem::path& vert_path,
        const std::filesystem::path& frag_path
    ) noexcept -> std::expected<shader, error> {
        auto vert_source = read_file(vert_path);
        if (not vert_source) return make_error(logic_error, "Failed to read vertex source file.");
        auto vert = dev.create_shader(*vert_source);
        if (not vert) return make_error(runtime_error, "Failed to make vertex shader module.");

        auto frag_source = read_file(frag_path);
        if (not frag_source) return make_error(logic_error, "Failed to read fragment source file.");
        auto frag = dev.create_shader(*frag_source);
        if (not frag) return make_error(runtime_error, "Failed to make fragment shader module.");

        return shader{std::move(*vert), std::move(*frag)};
    }

    [[nodiscard]] auto vertex() noexcept -> hgn::shader_module& { return vert_; }
    [[nodiscard]] auto fragment() noexcept -> hgn::shader_module& { return frag_; }

  private:
    shader(hgn::shader_module&& vert, hgn::shader_module&& frag) noexcept
        : vert_{std::move(vert)}, frag_{std::move(frag)} {}

    hgn::shader_module vert_;
    hgn::shader_module frag_;
};

class descriptor final {
  public:
    [[nodiscard]] static auto make_standard_descriptor(
        hgn::device& dev, hgn::buffer& uniform
    ) noexcept -> std::expected<descriptor, error> {
        auto layout = dev.create_descriptor_set_layout();
        if (not layout) return make_error(runtime_error, "Failed to make descriptor set layout.");

        auto pool = dev.create_descriptor_pool({hgn::descriptor_type::uniform_buffer});
        if (not pool) return make_error(runtime_error, "Failed to make descriptor pool.");

        auto set = pool->get_descriptor_set(*layout);
        if (not set) return make_error(runtime_error, "Failed to make descriptor set.");

        set->get_writer()
            .get_pusher()
            .buffer(uniform)
            .type(hgn::descriptor_type::uniform_buffer)
            .push()
            .write();

        return descriptor{std::move(*layout), std::move(*pool), *set};
    }

    [[nodiscard]] static auto make_texture_descriptor(
        hgn::device& dev, hgn::buffer& uniform, hgn::image_view& view, hgn::sampler& sampler
    ) noexcept -> std::expected<descriptor, error> {
        auto layout = dev.get_descriptor_set_layout_creator()
                          .get_binding_pusher()
                          .binding(0)
                          .shader_stage(hgn::shader_stage_flag_bits::vertex)
                          .type(hgn::descriptor_type::uniform_buffer)
                          .push()
                          .get_binding_pusher()
                          .binding(1)
                          .shader_stage(hgn::shader_stage_flag_bits::fragment)
                          .type(hgn::descriptor_type::combined_image_sampler)
                          .push()
                          .create();
        if (not layout) return make_error(runtime_error, "Failed to make descriptor set layout.");

        auto pool = dev.create_descriptor_pool(
            {hgn::descriptor_type::uniform_buffer, hgn::descriptor_type::combined_image_sampler}
        );

        auto set = pool->get_descriptor_set(*layout);
        if (not set) return make_error(runtime_error, "Failed to make descriptor set.");

        set->get_writer()
            .get_pusher()
            .binding(0)
            .buffer(uniform)
            .type(hgn::descriptor_type::uniform_buffer)
            .push()
            .get_pusher()
            .binding(1)
            .image(view, sampler)
            .type(hgn::descriptor_type::combined_image_sampler)
            .push()
            .write();

        return descriptor{std::move(*layout), std::move(*pool), *set};
    }

    [[nodiscard]] auto layout() noexcept -> hgn::descriptor_set_layout& { return layout_; }

    [[nodiscard]] auto set() noexcept -> hgn::descriptor_set& { return set_; }

  private:
    descriptor(
        hgn::descriptor_set_layout&& layout, hgn::descriptor_pool&& pool, hgn::descriptor_set set
    ) noexcept
        : layout_{std::move(layout)}, pool_{std::move(pool)}, set_{set} {}

    hgn::descriptor_set_layout layout_;
    hgn::descriptor_pool       pool_;
    hgn::descriptor_set        set_;
};

class texture_renderer final {
  public:
    [[nodiscard]] static auto make(
        hgn::allocator&      allocator,
        hgn::command_buffer& cmd,
        hgn::device&         dev,
        hgn::buffer&         uniform,
        const hgn::format    fmt
    ) noexcept -> std::expected<texture_renderer, error> {
        auto pixel = sbwi::create_image("../assets/test.jpg");
        if (not pixel) return make_error(runtime_error, "Failed to make pixel.");

        if (not cmd.begin()) return make_error(runtime_error, "Failed to begin command buffer.");

        auto image = allocator.create_image(pixel->width(), pixel->height());
        if (not image) return make_error(runtime_error, "Failed to make image.");

        auto staging = allocator.get_buffer_creator()
                           .usage(hgn::buffer_usage_flag_bits::transfer_src)
                           .size(672080)
                           .create();
        if (not staging) return make_error(runtime_error, "Failed to make staging buffer.");

        cmd.get_image_memory_barrier_executor()
            .image(*image)
            .layout(hgn::image_layout::undefined, hgn::image_layout::transfer_dst_optimal)
            .barrier();

        cmd.cpy_buffer_to_image(*staging, *image, pixel->width(), pixel->height());

        cmd.get_image_memory_barrier_executor()
            .image(*image)
            .layout(
                hgn::image_layout::transfer_dst_optimal, hgn::image_layout::shader_read_only_optimal
            )
            .barrier();

        if (not cmd.end()) return make_error(runtime_error, "Failed to end command buffer.");

        auto view =
            dev.get_image_view_creator().image(*image).format(hgn::format::r8g8b8a8_srgb).create();
        if (not view) return make_error(runtime_error, "Failed to make image view.");

        auto sampler = dev.create_sampler();
        if (not sampler) return make_error(runtime_error, "Failed to make sampler.");

        auto shader = shader::make(
            dev,
            "../include/sui/detail/shader/tex_vert.spv",
            "../include/sui/detail/shader/tex_frag.spv"
        );
        if (not shader) return make_error(runtime_error, "Failed to make shader", shader.error());

        auto descriptor = descriptor::make_texture_descriptor(dev, uniform, *view, *sampler);
        if (not descriptor)
            return make_error(runtime_error, "Failed to make descriptor.", descriptor.error());

        auto layout = dev.create_pipeline_layout(descriptor->layout());
        if (not layout) return make_error(runtime_error, "Failed to make pipeline layout.");

        auto pipe = dev.get_graphics_pipeline_creator()
                        .vertex_binding(sizeof(uv_vertex))
                        .get_vertex_attribute_pusher()
                        .location(0)
                        .offset(offsetof(uv_vertex, position))
                        .format(hgn::format::r32g32b32_sfloat)
                        .push()
                        .get_vertex_attribute_pusher()
                        .location(1)
                        .offset(offsetof(uv_vertex, color))
                        .format(hgn::format::r8g8b8a8_unorm)
                        .push()
                        .get_vertex_attribute_pusher()
                        .location(2)
                        .offset(offsetof(uv_vertex, tex_coord))
                        .format(hgn::format::r32g32_sfloat)
                        .push()
                        .color_attachment(fmt)
                        .layout(*layout)
                        .shaders(shader->vertex(), shader->fragment())
                        .create();
        if (not pipe) return make_error(runtime_error, "Failed to make pipeline.");

        return texture_renderer{
            std::move(*pixel),
            std::move(*image),
            std::move(*view),
            std::move(*sampler),
            std::move(*shader),
            std::move(*descriptor),
            std::move(*layout),
            std::move(*pipe)
        };
    }

    [[nodiscard]] auto pipeline_layout() noexcept -> hgn::pipeline_layout& { return layout_; }
    [[nodiscard]] auto pipeline() noexcept -> hgn::graphics_pipeline& { return pipe_; }
    [[nodiscard]] auto descriptor_set() noexcept -> hgn::descriptor_set& {
        return descriptor_.set();
    }

  private:
    texture_renderer(
        sbwi::image            pixel,
        hgn::image             image,
        hgn::image_view        view,
        hgn::sampler           sampler,
        shader                 shader,
        descriptor             descriptor,
        hgn::pipeline_layout   layout,
        hgn::graphics_pipeline pipe
    ) noexcept
        : pixel_{std::move(pixel)},
          image_{std::move(image)},
          view_{std::move(view)},
          sampler_{std::move(sampler)},
          shader_{std::move(shader)},
          descriptor_{std::move(descriptor)},
          layout_{std::move(layout)},
          pipe_{std::move(pipe)} {}

    sbwi::image            pixel_;
    hgn::image             image_;
    hgn::image_view        view_;
    hgn::sampler           sampler_;
    shader                 shader_;
    descriptor             descriptor_;
    hgn::pipeline_layout   layout_;
    hgn::graphics_pipeline pipe_;
};

class renderer final {
  public:
    [[nodiscard]] static auto make(
        hardware& hard, const hgn::format fmt, hgn::command_buffer& cmd
    ) noexcept -> std::expected<renderer, error> {
        auto buffer = buffer::make(hard);
        if (not buffer) return make_error(runtime_error, "Failed to make buffer.", buffer.error());

        auto& dev = hard.device();

        auto shader = shader::make(
            dev, "../include/sui/detail/shader/vert.spv", "../include/sui/detail/shader/frag.spv"
        );
        if (not shader) return make_error(runtime_error, "Failed to make shader", shader.error());

        auto descriptor = descriptor::make_standard_descriptor(dev, buffer->uniform());
        if (not descriptor)
            return make_error(runtime_error, "Failed to make descriptor", descriptor.error());

        auto layout = dev.create_pipeline_layout(descriptor->layout());
        if (not layout) return make_error(runtime_error, "Failed to make pipeline layout");

        auto pipe = dev.get_graphics_pipeline_creator()
                        .vertex_binding(sizeof(vertex))
                        .get_vertex_attribute_pusher()
                        .location(0)
                        .offset(offsetof(vertex, position))
                        .format(hgn::format::r32g32b32_sfloat)
                        .push()
                        .get_vertex_attribute_pusher()
                        .location(1)
                        .offset(offsetof(vertex, color))
                        .format(hgn::format::r8g8b8a8_unorm)
                        .push()
                        .color_attachment(fmt)
                        .layout(*layout)
                        .shaders(shader->vertex(), shader->fragment())
                        .create();

        if (not pipe) return make_error(runtime_error, "Failed to make graphics pipeline.");

        auto tex = texture_renderer::make(buffer->allocator(), cmd, dev, buffer->uniform(), fmt);
        if (not tex)
            return make_error(runtime_error, "Failed to make texture renderer.", tex.error());

        return renderer{
            std::move(*buffer),
            std::move(*shader),
            std::move(*layout),
            std::move(*pipe),
            std::move(*descriptor),
            std::move(*tex)
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

    [[nodiscard]] auto texture() noexcept -> texture_renderer& { return tex_; }

  private:
    renderer(
        buffer&&                 buffer,
        shader&&                 shader,
        hgn::pipeline_layout&&   layout,
        hgn::graphics_pipeline&& pipe,
        descriptor&&             descriptor,
        texture_renderer&&       tex
    ) noexcept
        : buffer_{std::move(buffer)},
          shader_{std::move(shader)},
          descriptor_{std::move(descriptor)},
          layout_{std::move(layout)},
          pipe_{std::move(pipe)},
          tex_{std::move(tex)} {}

    buffer                 buffer_;
    shader                 shader_;
    descriptor             descriptor_;
    hgn::pipeline_layout   layout_;
    hgn::graphics_pipeline pipe_;
    texture_renderer       tex_;
};
}  // namespace sui::detail