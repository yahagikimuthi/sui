#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include <hgn/descriptor.hpp>
#include <hgn/enums.hpp>
#include <hgn/rendering.hpp>

namespace sui {
struct vertex_attribute_info final {
    std::uint32_t location;
    std::uint32_t offset;
    hgn::format   format;
};

struct graphics_pipeline_create_info final {
    std::uint32_t                        vertex_struct_size;
    std::vector<vertex_attribute_info>   vertex_attributes;
    std::optional<hgn::pipeline_layout&> layout;
    std::optional<hgn::shader_module&>   vertex_shader;
    std::optional<hgn::shader_module&>   fragment_shader;
};

struct push_constant_info final {
    std::uint32_t               offset;
    hgn::shader_stage_flag_bits stage_flags;
    std::uint32_t               size;
};

struct pipeline_layout_create_info final {
    std::vector<std::reference_wrapper<hgn::descriptor_set_layout>> layouts;
    std::vector<push_constant_info>                                 push_constants;
};

struct descriptor_set_layout_binding final {
    std::uint32_t               binding;
    hgn::descriptor_type        type;
    hgn::shader_stage_flag_bits shader_stage;
};
}  // namespace sui