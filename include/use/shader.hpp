#pragma once

#include <array>
#include <expected>
#include <optional>
#include <utility>
#include <vector>

#include <akr/akr.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/rendering.hpp>

#include "sui/detail/others.hpp"
#include "sui/info.hpp"
#include "sui/sui.hpp"
#include "use/structs.hpp"

[[nodiscard]] inline auto create_shader_module(
    const std::filesystem::path& path, sui::engine& engine
) noexcept -> hgn::shader_module {
    return *sui::detail::read_file(path)
                .transform([&](std::vector<char> code) noexcept -> auto {
                    return engine.create_shader(code);
                })
                .and_then(
                    [](std::expected<hgn::shader_module, hgn::result> e
                    ) noexcept -> std::optional<hgn::shader_module> {
                        return not e ? std::nullopt : std::optional{std::move(*e)};
                    }
                );
}

[[nodiscard]] inline auto create_standard_shader(sui::engine& engine) noexcept -> shader {
    auto vert = create_shader_module("../include/sui/detail/shader/vert.spv", engine);
    auto frag = create_shader_module("../include/sui/detail/shader/frag.spv", engine);

    auto layout =
        *engine.create_descriptor_set_layout(std::array{sui::descriptor_set_layout_binding{
            .binding      = 0,
            .type         = hgn::descriptor_type::uniform_buffer,
            .shader_stage = hgn::shader_stage_flag_bits::vertex
        }});

    return shader{
        .vertex = std::move(vert), .fragment = std::move(frag), .layout = std::move(layout)
    };
}