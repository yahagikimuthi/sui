#pragma once

#include <cassert>
#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <sui/sui.hpp>

#include "sui/info.hpp"
#include "use/shader.hpp"
#include "use/structs.hpp"

[[nodiscard]] inline auto create_pipeline(sui::engine& engine) -> pipeline {
    auto shader = create_standard_shader(engine);

    auto pool = *engine.create_descriptor_pool({hgn::descriptor_type::uniform_buffer});

    auto layout_info = sui::pipeline_layout_create_info{};
    layout_info.layouts.emplace_back(shader.layout);
    layout_info.push_constants.emplace_back(sui::push_constant_info{
        .offset = 0, .stage_flags = hgn::shader_stage_flag_bits::vertex, .size = sizeof(glm::mat4)
    });

    auto layout = *engine.create_pipeline_layout(layout_info);

    auto pipe_info = sui::graphics_pipeline_create_info{
        .vertex_struct_size = sizeof(vertex),
        .vertex_attributes =
            {sui::vertex_attribute_info{
                 .location = 0,
                 .offset   = offsetof(vertex, position),
                 .format   = hgn::format::r32g32b32_sfloat
             },
             sui::vertex_attribute_info{
                 .location = 1,
                 .offset   = offsetof(vertex, normal),
                 .format   = hgn::format::r32g32b32_sfloat
             },
             sui::vertex_attribute_info{
                 .location = 2,
                 .offset   = offsetof(vertex, color),
                 .format   = hgn::format::r8g8b8a8_unorm
             }},
        .layout          = layout,
        .vertex_shader   = shader.vertex,
        .fragment_shader = shader.fragment
    };
    auto pipe = *engine.create_graphics_pipeline(pipe_info);

    return pipeline{
        .shader = std::move(shader),
        .pool   = std::move(pool),
        .layout = std::move(layout),
        .pipe   = std::move(pipe)
    };
}