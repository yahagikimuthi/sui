#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/rendering.hpp>

#include "sui/shader.hpp"
#include "sui/vertex.hpp"

namespace sui {
class pipeline final {
  public:
    [[nodiscard]] static auto create_standards(
        hgn::device& dev, const hgn::format swapchain_format
    ) noexcept -> std::expected<pipeline, akr::error> {
        auto shader_obj = shader::create_standard(dev);
        if (not shader_obj)
            return shader_obj.error().rethrow_runtime("Failed to create shader class.");

        auto pool = dev.create_descriptor_pool({hgn::descriptor_type::uniform_buffer});
        if (not pool) return akr::throw_runtime("Failed to create descriptor pool.");

        auto layout = dev.create_pipeline_layout(shader_obj->layout());
        if (not layout) return akr::throw_runtime("Failed to create pipeline layout.");

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
                        .color_attachment(swapchain_format)
                        .layout(*layout)
                        .shaders(shader_obj->vertex_shader(), shader_obj->fragment_shader())
                        .create();
        if (not pipe) return akr::throw_runtime("Failed to create graphics pipeline.");

        return pipeline{
            std::move(*shader_obj), std::move(*pool), std::move(*layout), std::move(*pipe)
        };
    }

    [[nodiscard]] static auto create_textures(
        hgn::device& dev, const hgn::format swapchain_format
    ) noexcept -> std::expected<pipeline, akr::error> {
        auto shader_obj = shader::create_texture(dev);
        if (not shader_obj)
            return shader_obj.error().rethrow_runtime("Failed to create shader class.");

        auto pool = dev.create_descriptor_pool(
            {hgn::descriptor_type::uniform_buffer, hgn::descriptor_type::combined_image_sampler}
        );
        if (not pool) return akr::throw_runtime("Failed to create descriptor pool.");

        auto layout = dev.create_pipeline_layout(shader_obj->layout());
        if (not layout) return akr::throw_runtime("Failed to create pipeline layout.");

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
                        .color_attachment(swapchain_format)
                        .layout(*layout)
                        .shaders(shader_obj->vertex_shader(), shader_obj->fragment_shader())
                        .create();
        if (not pipe) return akr::throw_runtime("Failed to create pipeline.");

        return pipeline{
            std::move(*shader_obj), std::move(*pool), std::move(*layout), std::move(*pipe)
        };
    }

  private:
    pipeline(
        shader                 shader,
        hgn::descriptor_pool   pool,
        hgn::pipeline_layout   layout,
        hgn::graphics_pipeline pipe
    ) noexcept
        : shader_{std::move(shader)},
          pool_{std::move(pool)},
          layout_{std::move(layout)},
          pipe_{std::move(pipe)} {}

    shader                 shader_;
    hgn::descriptor_pool   pool_;
    hgn::pipeline_layout   layout_;
    hgn::graphics_pipeline pipe_;
};

class pipeline_container final {
  public:
    [[nodiscard]] static auto create(hgn::device& dev, const hgn::format swapchain_format) noexcept
        -> std::expected<pipeline_container, akr::error> {
        auto standard = pipeline::create_standards(dev, swapchain_format);
        if (not standard)
            return standard.error().rethrow_runtime("Failed to create standard pipeline");

        auto texture = pipeline::create_textures(dev, swapchain_format);
        if (not texture)
            return texture.error().rethrow_runtime("Failed to create texture pipeline.");

        return pipeline_container{std::move(*standard), std::move(*texture)};
    }

  private:
    pipeline_container(pipeline standard, pipeline texture) noexcept
        : standard_{std::move(standard)}, texture_{std::move(texture)} {}

    pipeline standard_;
    pipeline texture_;
};
}  // namespace sui