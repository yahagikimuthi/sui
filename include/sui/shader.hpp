#pragma once

#include <expected>
#include <filesystem>
#include <optional>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/buffer.hpp>
#include <hgn/descriptor.hpp>
#include <hgn/descriptor_pool.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/rendering.hpp>

#include "sui/detail/others.hpp"
#include "sui/vertex.hpp"

namespace sui {
class shader_modules final {
  public:
    [[nodiscard]] static auto create(
        hgn::device&                 dev,
        const std::filesystem::path& vert_source,
        const std::filesystem::path& frag_source
    ) noexcept -> std::expected<shader_modules, akr::error> {
        auto vert_code = detail::read_file(vert_source);
        if (not vert_code)
            return akr::make_error(akr::logic_error, "Failed to read vertex shader code.");
        auto vertex = dev.create_shader(*vert_code);
        if (not vertex)
            return akr::make_error(akr::runtime_error, "Failed to create vertex shader module.");

        auto frag_code = detail::read_file(frag_source);
        if (not frag_code)
            return akr::make_error(akr::logic_error, "Failed to read fragment shader code.");
        auto frag = dev.create_shader(*frag_code);
        if (not frag)
            return akr::make_error(akr::runtime_error, "Failed to create fragment shader module.");

        return shader_modules{std::move(*vertex), std::move(*frag)};
    }

    [[nodiscard]] auto vertex() noexcept -> hgn::shader_module& { return vertex_; }
    [[nodiscard]] auto fragment() noexcept -> hgn::shader_module& { return fragment_; }

  private:
    shader_modules(hgn::shader_module vertex, hgn::shader_module fragment) noexcept
        : vertex_{std::move(vertex)}, fragment_{std::move(fragment)} {}

    hgn::shader_module vertex_;
    hgn::shader_module fragment_;
};

class shader final {
  public:
    [[nodiscard]] static auto create_standard(hgn::device& dev
    ) noexcept -> std::expected<shader, akr::error> {
        auto shaders = shader_modules::create(
            dev, "../include/sui/detail/shader/vert.spv", "../include/sui/detail/shader/frag.spv"
        );
        if (not shaders)
            return akr::make_error(
                akr::runtime_error, "Failed to create shader modules.", shaders.error()
            );

        auto layout = dev.get_descriptor_set_layout_creator()
                          .get_binding_pusher()
                          .binding(0)
                          .shader_stage(hgn::shader_stage_flag_bits::vertex)
                          .type(hgn::descriptor_type::uniform_buffer)
                          .push()
                          .create();
        if (not layout)
            return akr::make_error(akr::runtime_error, "Failed to create descriptor layout.");

        return shader{std::move(*shaders), std::move(*layout)};
    }

    [[nodiscard]] auto vertex_shader() noexcept -> hgn::shader_module& { return shaders_.vertex(); }
    [[nodiscard]] auto fragment_shader() noexcept -> hgn::shader_module& {
        return shaders_.fragment();
    }
    [[nodiscard]] auto layout() noexcept -> hgn::descriptor_set_layout& { return layout_; }

  private:
    shader(shader_modules shaders, hgn::descriptor_set_layout layout) noexcept
        : shaders_{std::move(shaders)}, layout_{std::move(layout)} {}

    shader_modules             shaders_;
    hgn::descriptor_set_layout layout_;
};

class standard_pipeline final {
  public:
    [[nodiscard]] static auto create(hgn::device& dev, const hgn::format surface_format) noexcept
        -> std::expected<standard_pipeline, akr::error> {
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

        return standard_pipeline{
            std::move(*shader_obj), std::move(*pool), std::move(*layout), std::move(*pipe)
        };
    }

  private:
    standard_pipeline(
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