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

namespace sui {
class shader_modules final {
  public:
    [[nodiscard]] static auto create(
        hgn::device&                 dev,
        const std::filesystem::path& vert_source,
        const std::filesystem::path& frag_source
    ) noexcept -> std::expected<shader_modules, akr::error> {
        auto vert_code = detail::read_file(vert_source);
        if (not vert_code) return akr::throw_logic("Failed to read vertex shader code.");
        auto vertex = dev.create_shader(*vert_code);
        if (not vertex) return akr::throw_runtime("Failed to create vertex shader module.");

        auto frag_code = detail::read_file(frag_source);
        if (not frag_code) return akr::throw_logic("Failed to read fragment shader code.");
        auto frag = dev.create_shader(*frag_code);
        if (not frag) return akr::throw_runtime("Failed to create fragment shader module.");

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
        if (not shaders) return shaders.error().rethrow_runtime("Failed to create shader modules.");

        auto layout = dev.get_descriptor_set_layout_creator()
                          .get_binding_pusher()
                          .binding(0)
                          .shader_stage(hgn::shader_stage_flag_bits::vertex)
                          .type(hgn::descriptor_type::uniform_buffer)
                          .push()
                          .create();
        if (not layout) return akr::throw_runtime("Failed to create descriptor layout.");

        return shader{std::move(*shaders), std::move(*layout)};
    }

    [[nodiscard]] static auto create_texture(hgn::device& dev
    ) noexcept -> std::expected<shader, akr::error> {
        auto shaders = shader_modules::create(
            dev,
            "../include/sui/detail/shader/tex_vert.spv",
            "../include/sui/detail/shader/tex_frag.spv"
        );
        if (not shaders) return shaders.error().rethrow_runtime("Failed to create shader modules.");

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
        if (not layout) return akr::throw_runtime("Failed to create layout.");

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
}  // namespace sui