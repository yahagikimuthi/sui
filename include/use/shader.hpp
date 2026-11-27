#pragma once

#include <expected>
#include <optional>
#include <utility>
#include <vector>

#include <akr/akr.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/rendering.hpp>

#include "sui/detail/others.hpp"
#include "use/structs.hpp"

[[nodiscard]] inline auto create_shader_module(
    const std::filesystem::path& path, hgn::device& dev
) noexcept -> hgn::shader_module {
    return *sui::detail::read_file(path)
                .transform([&](std::vector<char> code) noexcept -> auto {
                    return dev.create_shader(code);
                })
                .and_then(
                    [](std::expected<hgn::shader_module, hgn::result> e
                    ) noexcept -> std::optional<hgn::shader_module> {
                        return not e ? std::nullopt : std::optional{std::move(*e)};
                    }
                );
}

[[nodiscard]] inline auto create_standard_shader(hgn::device& dev) noexcept -> shader {
    auto vert = create_shader_module("../include/sui/detail/shader/vert.spv", dev);
    auto frag = create_shader_module("../include/sui/detail/shader/frag.spv", dev);

    auto layout = *dev.get_descriptor_set_layout_creator()
                       .get_binding_pusher()
                       .binding(0)
                       .type(hgn::descriptor_type::uniform_buffer)
                       .shader_stage(hgn::shader_stage_flag_bits::vertex)
                       .push()
                       .create();

    return shader{
        .vertex = std::move(vert), .fragment = std::move(frag), .layout = std::move(layout)
    };
}

[[nodiscard]] inline auto create_texture_shader(hgn::device& dev) noexcept -> shader {
    auto vert = create_shader_module("../include/sui/detail/shader/tex_vert.spv", dev);
    auto frag = create_shader_module("../include/sui/detail/shader/tex_frag.spv", dev);

    auto layout = *dev.get_descriptor_set_layout_creator()
                       .get_binding_pusher()
                       .binding(0)
                       .type(hgn::descriptor_type::uniform_buffer)
                       .shader_stage(hgn::shader_stage_flag_bits::vertex)
                       .push()
                       .get_binding_pusher()
                       .binding(1)
                       .type(hgn::descriptor_type::combined_image_sampler)
                       .shader_stage(hgn::shader_stage_flag_bits::fragment)
                       .push()
                       .create();
    return shader{
        .vertex = std::move(vert), .fragment = std::move(frag), .layout = std::move(layout)
    };
}