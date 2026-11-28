#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include <hgn/enums.hpp>
#include <hgn/rendering.hpp>

namespace sui {
struct vertex_attribute_info final {
    std::uint32_t location;
    std::uint32_t offset;
    hgn::format   format;
};

struct graphics_pipeline_create_info final {
    std::uint32_t                                           vertex_structure_stride;
    std::vector<vertex_attribute_info>                      vertex_attributes;
    hgn::format                                             format;
    hgn::pipeline_layout                                    layout;
    std::vector<std::reference_wrapper<hgn::shader_module>> shaders_;
};
}  // namespace sui