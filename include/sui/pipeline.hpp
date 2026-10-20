#pragma once

#include <array>
#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/vertex.hpp>

#include "sui/error.hpp"

namespace sui {
class pipeline final {
    struct vertex final {
        float position[2];  // NOLINT
        float color[3];     // NOLINT
    };

  public:
    [[nodiscard]] static auto try_make(hgn::device& dev, const hgn::format fmt) noexcept
        -> std::expected<pipeline, error> {
        const auto binding_description =
            hgn::vertex_input_binding_description{}.stride(sizeof(float) * 5);

        const auto attribute_descriptions = std::array<hgn::vertex_input_attribute_description, 2>{
            hgn::vertex_input_attribute_description{}
                .location(0)
                .format(hgn::format::r32g32_sfloat)
                .offset(offsetof(vertex, position)),
            hgn::vertex_input_attribute_description{}
                .location(1)
                .format(hgn::format::r32g32_sfloat)
                .offset(offsetof(vertex, color))
        };

        const auto vertex_input_info = hgn::pipeline_vertex_input_state_create_info{}
                                           .vertex_binding_description(binding_description)
                                           .vertex_attribute_descriptions(attribute_descriptions);

        const auto input_assembly = hgn::pipeline_input_assembly_state_create_info{};

        const auto dynamic_states = std::array<hgn::dynamic_state, 2>{
            hgn::dynamic_state::viewport, hgn::dynamic_state::scissor
        };

        const auto dynamic_state =
            hgn::pipeline_dynamic_state_create_info{}.dynamic_states(dynamic_states);

        const auto viewport_state = hgn::pipeline_viewport_state_create_info{};

        const auto rasterizer = hgn::pipeline_rasterization_state_create_info{};

        const auto multisampling = hgn::pipeline_multisample_state_create_info{};

        const auto color_blend_attachment = hgn::pipeline_color_blend_attachment_state{};

        const auto color_blending =
            hgn::pipeline_color_blend_state_create_info{}.attachment(color_blend_attachment);

        // TODO Moduleをattach
        const auto vert_shader_stage_info =
            hgn::pipeline_shader_stage_create_info{}.stage(hgn::shader_stage_flag_bits::vertex);

        const auto frag_shader_stage_info =
            hgn::pipeline_shader_stage_create_info{}.stage(hgn::shader_stage_flag_bits::fragment);

        const auto shader_stages = std::array<hgn::pipeline_shader_stage_create_info, 2>{
            vert_shader_stage_info, frag_shader_stage_info
        };

        const auto pipeline_layout_info = hgn::pipeline_layout_create_info{};

        const auto pipe_layout = hgn::try_make_pipeline_layout(dev, pipeline_layout_info);
        if (not pipe_layout) return make_error(runtime_error, "Failed to make pipeline layout.");

        const auto pipeline_rendering_info =
            hgn::pipeline_rendering_create_info{}.color_attachment_format(fmt);

        const auto pipeline_info = hgn::graphics_pipeline_create_info{}
                                       .rendering_info(pipeline_rendering_info)
                                       .stages(shader_stages)
                                       .vertex_input_state(vertex_input_info)
                                       .input_assembly_state(input_assembly)
                                       .viewport_state(viewport_state)
                                       .rasterization_state(rasterizer)
                                       .multisample_state(multisampling)
                                       .color_blend_state(color_blending)
                                       .dynamic_states(dynamic_state)
                                       .layout(pipe_layout->get());

        const auto res = hgn::try_make_graphics_pipeline(dev, pipeline_info);
        if (not res) return make_error(runtime_error, "Failed to make graphics pipeline.");

        return pipeline{res->get(), dev};
    }

    pipeline(const pipeline&) noexcept = delete;

    auto operator=(const pipeline&) noexcept -> pipeline& = delete;

    pipeline(pipeline&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(pipeline&& other) noexcept -> pipeline& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;

        return *this;
    }

    ~pipeline() noexcept { destroy(); }

  private:
    explicit pipeline(hgn::pipeline& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_pipeline(dev_.get(), *native_);
    }

    std::optional<hgn::pipeline&>       native_;
    std::reference_wrapper<hgn::device> dev_;
};
}  // namespace sui