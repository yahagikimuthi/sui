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
    [[nodiscard]] static auto create(hgn::device& dev, const hgn::format surface_format) noexcept
        -> std::expected<pipeline, akr::error> {
        auto shader_obj = shader::create_standard(dev);
        if (not shader_obj)
            return akr::make_error(akr::runtime_error, "Failed to create shader class.");

        auto pool = dev.create_descriptor_pool({hgn::descriptor_type::uniform_buffer});
        if (not pool)
            return akr::make_error(akr::runtime_error, "Failed to create descriptor pool.");

        auto layout = dev.create_pipeline_layout(shader_obj->layout());
        if (not layout)
            return akr::make_error(akr::runtime_error, "Failed to create pipeline layout.");

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
                        .color_attachment(surface_format)
                        .layout(*layout)
                        .shaders(shader_obj->vertex_shader(), shader_obj->fragment_shader())
                        .create();
        if (not pipe)
            return akr::make_error(akr::runtime_error, "Failed to create graphics pipeline.");

        return pipeline{
            std::move(*shader_obj), std::move(*pool), std::move(*layout), std::move(*pipe)
        };
    }

  private:
    pipeline(
        shader                 shader,
        hgn::descriptor_pool   pool,
        hgn::pipeline_layout   layout,
        hgn::graphics_pipeline pipeline
    ) noexcept
        : shader_{std::move(shader)},
          pool_{std::move(pool)},
          layout_{std::move(layout)},
          pipe_{std::move(pipeline)} {}

    shader                 shader_;
    hgn::descriptor_pool   pool_;
    hgn::pipeline_layout   layout_;
    hgn::graphics_pipeline pipe_;
};
}  // namespace sui