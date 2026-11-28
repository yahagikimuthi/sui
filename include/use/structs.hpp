#pragma once

#include <cassert>

#include <hgn/allocator.hpp>
#include <hgn/buffer.hpp>
#include <hgn/command.hpp>
#include <hgn/descriptor.hpp>
#include <hgn/descriptor_pool.hpp>
#include <hgn/device.hpp>
#include <hgn/rendering.hpp>

#include "sui/detail/others.hpp"
#include "sui/vertex.hpp"

struct buffers final {
    hgn::buffer         vertex;
    hgn::buffer         index;
    hgn::buffer         uniform;
    hgn::descriptor_set set;
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

struct shadow final {
    hgn::image      depth_img;
    hgn::image_view depth_view;
    hgn::sampler    sampler;
};

struct push_constants final {
    alignas(16) glm::mat4 model{1.f};
};

using sui::uv_vertex;                      // NOLINT
using sui::vertex;                         // NOLINT
using sui::detail::uniform_buffer_object;  // NOLINT