#pragma once

#include <cassert>
#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>

#include "use/shader.hpp"
#include "use/structs.hpp"

[[nodiscard]] inline auto create_standard_pipeline(
    hgn::device& dev, const hgn::format swapchain_fmt, const hgn::format depth_fmt
) -> pipeline {
    auto shader = create_standard_shader(dev);
    assert(shader and "Failed to create shader.");

    auto pool = dev.create_descriptor_pool({hgn::descriptor_type::uniform_buffer});
    assert(pool and "Failed to create descriptor pool");

    auto layout = dev.create_pipeline_layout(shader->layout);
    assert(layout and "Failed to create pipeline layout.");

    auto pipe = dev.get_graphics_pipeline_creator()
                    .vertex_binding(sizeof(vertex))
                    .get_vertex_attribute_pusher()
                    .location(0)
                    .offset(offsetof(vertex, position))
                    .format(hgn::format::r32g32_sfloat)
                    .push()
                    .get_vertex_attribute_pusher()
                    .location(1)
                    .offset(offsetof(vertex, color))
                    .format(hgn::format::r8g8b8a8_unorm)
                    .push()
                    .color_attachment(swapchain_fmt)
                    .depth_attachment(depth_fmt)
                    .layout(*layout)
                    .shaders(shader->vertex, shader->fragment)
                    .create();
    assert(pipe and "Failed to create graphics pipeline.");

    return pipeline{
        .shader = std::move(*shader),
        .pool   = std::move(*pool),
        .layout = std::move(*layout),
        .pipe   = std::move(*pipe)
    };
}

[[nodiscard]] inline auto create_texture_pipeline(
    hgn::device& dev, const hgn::format swapchain_fmt, const hgn::format depth_fmt
) -> pipeline {
    auto shader = create_texture_shader(dev);
    assert(shader and "Failed to create standard shader.");

    auto pool = dev.create_descriptor_pool(
        {hgn::descriptor_type::uniform_buffer, hgn::descriptor_type::combined_image_sampler}
    );
    assert(pool and "Failed to create descriptor pool");

    auto layout = dev.create_pipeline_layout(shader->layout);
    assert(layout and "Failed to create pipeline layout.");

    auto pipe = dev.get_graphics_pipeline_creator()
                    .vertex_binding(sizeof(uv_vertex))
                    .get_vertex_attribute_pusher()
                    .location(0)
                    .offset(offsetof(uv_vertex, position))
                    .format(hgn::format::r32g32b32_sfloat)
                    .push()
                    .get_vertex_attribute_pusher()
                    .location(1)
                    .offset(offsetof(vertex, color))
                    .format(hgn::format::r8g8b8a8_unorm)
                    .push()
                    .get_vertex_attribute_pusher()
                    .location(2)
                    .offset(offsetof(uv_vertex, tex_coord))
                    .format(hgn::format::r32g32_sfloat)
                    .push()
                    .color_attachment(swapchain_fmt)
                    .depth_attachment(depth_fmt)
                    .layout(*layout)
                    .shaders(shader->vertex, shader->fragment)
                    .create();
    assert(pipe and "Failed to create graphics pipeline.");

    return pipeline{
        .shader = std::move(*shader),
        .pool   = std::move(*pool),
        .layout = std::move(*layout),
        .pipe   = std::move(*pipe)
    };
}