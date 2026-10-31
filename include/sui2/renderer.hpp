#pragma once

#include <expected>
#include <utility>

#include <hgn2/enums.hpp>
#include <hgn2/pipeline.hpp>
#include <hgn2/pipeline_layout.hpp>
#include <hgn2/pipeline_structs.hpp>
#include <hgn2/shader.hpp>

#include "sui2/error.hpp"
#include "sui2/hardware.hpp"
#include "sui2/others.hpp"
#include "sui2/vertex.hpp"

namespace sui {
class renderer final {
  public:
    [[nodiscard]] static auto make(hardware& hard, const hgn::format fmt) noexcept
        -> std::expected<renderer, error> {
        auto&      dev         = hard.device();
        const auto vert_source = read_file("../include/sui2/vert.spv");
        if (not vert_source) return make_error(logic_error, "Failed to read vertex source file.");
        const auto frag_source = read_file("../include/sui2/frag.spv");
        if (not frag_source) return make_error(logic_error, "Failed to read fragment source file.");

        auto vert = dev.make_shader(*vert_source);
        auto frag = dev.make_shader(*frag_source);

        if (not vert) return make_error(runtime_error, "Failed to make vertex shader.");
        if (not frag) return make_error(runtime_error, "Failed to make fragment shader.");

        auto layout = dev.get_pipeline_layout();
        if (not layout) return make_error(runtime_error, "Failed to make pipeline layout");

        auto pipe = dev.get_graphics_pipeline_builder()
                        .vertex_input_state(
                            hgn::pipeline_vertex_input_state::builder{}
                                .binding(sizeof(float) * 5)
                                .offsets(offsetof(vertex, position), offsetof(vertex, color))
                                .build()
                        )
                        .color_attachment(fmt)
                        .layout(*layout)
                        .shaders(*vert, *frag)
                        .build();

        if (not pipe) return make_error(runtime_error, "Failed to make graphics pipeline.");

        return renderer{std::move(*vert), std::move(*frag), std::move(*layout), std::move(*pipe)};
    }

  private:
    explicit renderer(
        hgn::shader_module&&     vert,
        hgn::shader_module&&     frag,
        hgn::pipeline_layout&&   layout,
        hgn::graphics_pipeline&& pipe
    ) noexcept
        : vert_{std::move(vert)},
          frag_{std::move(frag)},
          pipe_layout_{std::move(layout)},
          pipe_{std::move(pipe)} {}

    hgn::shader_module     vert_;
    hgn::shader_module     frag_;
    hgn::pipeline_layout   pipe_layout_;
    hgn::graphics_pipeline pipe_;
};
}  // namespace sui