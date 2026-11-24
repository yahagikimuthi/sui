#pragma once

#include <cassert>
#include <exception>
#include <utility>

#include <hgn/allocator.hpp>
#include <hgn/buffer.hpp>
#include <hgn/command.hpp>
#include <hgn/descriptor.hpp>
#include <hgn/descriptor_pool.hpp>
#include <hgn/device.hpp>
#include <hgn/rendering.hpp>
#include <sbwi/sbwi.hpp>

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

struct texture final {
    sbwi::image         pixel;
    hgn::image          image;
    hgn::image_view     view;
    hgn::sampler        sampler;
    hgn::descriptor_set set;
};

struct context final {
    hgn::device&         dev;
    hgn::command_buffer& cmd;
    hgn::allocator&      allocator;
};

template <typename T>
[[nodiscard]] inline auto unwrap(T&& t) noexcept -> auto {
    if (not t) std::terminate();
    return *std::forward<T>(t);
}

struct push_constants final {
    glm::mat4 model{1.f};
};

using sui::uv_vertex;                      // NOLINT
using sui::vertex;                         // NOLINT
using sui::detail::uniform_buffer_object;  // NOLINT