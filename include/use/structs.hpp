#pragma once

#include <hgn/buffer.hpp>
#include <hgn/descriptor.hpp>
#include <hgn/rendering.hpp>

#include "hgn/descriptor_pool.hpp"
#include "sui/detail/others.hpp"
#include "sui/vertex.hpp"

struct buffers final {
    hgn::buffer vertex;
    hgn::buffer index;
    hgn::buffer uniform;
};

struct shader final {
    hgn::shader_module         vertex;
    hgn::shader_module         fragment;
    hgn::descriptor_set_layout layout;
};

struct pipeline final {
    struct shader          shader;
    hgn::descriptor_pool   pool;
    hgn::pipeline_layout   layout;
    hgn::graphics_pipeline pipe;
};

using sui::uv_vertex;                      // NOLINT
using sui::vertex;                         // NOLINT
using sui::detail::uniform_buffer_object;  // NOLINT