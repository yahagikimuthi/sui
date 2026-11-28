#pragma once

#include <vulkan/vulkan_core.h>
#include <cassert>
#include <expected>

#include <akr/akr.hpp>
#include <hgn/allocator.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>

#include "hgn/descriptor.hpp"
#include "sui/sui.hpp"
#include "use/structs.hpp"

[[nodiscard]] inline auto create_buffers(sui::engine& engine, pipeline& pipe) noexcept -> buffers {
    auto vertex = *engine.create_buffer(10000, hgn::buffer_usage_flag_bits::vertex);

    auto index = *engine.create_buffer(10000, hgn::buffer_usage_flag_bits::index);

    auto uniform = *engine.create_buffer(10000, hgn::buffer_usage_flag_bits::uniform);

    auto set = *pipe.pool.get_descriptor_set(pipe.shader.layout);
    set.get_writer()
        .get_pusher()
        .binding(0)
        .buffer(uniform)
        .type(hgn::descriptor_type::uniform_buffer)
        .push()
        .write();

    return buffers{
        .vertex  = std::move(vertex),
        .index   = std::move(index),
        .uniform = std::move(uniform),
        .set     = set
    };
}