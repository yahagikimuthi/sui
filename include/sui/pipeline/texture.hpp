#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/descriptor_pool.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <hgn/rendering.hpp>

#include "sui/shader.hpp"
#include "sui/vertex.hpp"

namespace sui {
class texture_pipeline final {
  public:
    [[nodiscard]] static auto create(hgn::device& dev, const hgn::format surface_format) noexcept
        -> std::expected<texture_pipeline, akr::error> {
        auto shader_obj = shader::create_texture(dev);
        if (not shader_obj)
            return akr::make_error(
                akr::runtime_error, "Failed to create shader class.", shader_obj.error()
            );

        auto pool = dev.create_descriptor_pool(
            {hgn::descriptor_type::uniform_buffer, hgn::descriptor_type::combined_image_sampler}
        );
        if (not pool)
            return akr::make_error(akr::runtime_error, "Failed to create descriptor pool.");

        auto layout = dev.create_pipeline_layout(shader_obj->layout());
        if (not layout)
            return akr::make_error(akr::runtime_error, "Failed to create pipeline layout.");

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
                        .color_attachment(surface_format)
                        .layout(*layout)
                        .shaders(shader_obj->vertex_shader(), shader_obj->fragment_shader())
                        .create();
        if (not pipe) return akr::make_error(akr::runtime_error, "Failed to create pipeline.");

        auto sampler = dev.create_sampler();
        if (not sampler) return akr::make_error(akr::runtime_error, "Failed to create sampler.");

        return texture_pipeline{
            std::move(*shader_obj),
            std::move(*pool),
            std::move(*layout),
            std::move(*pipe),
            std::move(*sampler)
        };
    }

  private:
    texture_pipeline(
        shader                 shader,
        hgn::descriptor_pool   pool,
        hgn::pipeline_layout   layout,
        hgn::graphics_pipeline pipe,
        hgn::sampler           sampler
    ) noexcept
        : shader_{std::move(shader)},
          pool_{std::move(pool)},
          layout_{std::move(layout)},
          pipe_{std::move(pipe)},
          sampler_{std::move(sampler)} {}

    shader                 shader_;
    hgn::descriptor_pool   pool_;
    hgn::pipeline_layout   layout_;
    hgn::graphics_pipeline pipe_;
    hgn::sampler           sampler_;
};
}  // namespace sui