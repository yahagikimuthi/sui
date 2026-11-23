#pragma once

#include <vulkan/vulkan_core.h>
#include <cassert>
#include <expected>

#include <akr/akr.hpp>
#include <hgn/allocator.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>

#include "use/structs.hpp"

[[nodiscard]] inline auto create_buffers(hgn::allocator& allocator) noexcept -> buffers {
    auto vertex = allocator.get_buffer_creator()
                      .size(10000)
                      .usage(hgn::buffer_usage_flag_bits::vertex)
                      .create();
    assert(vertex and "Failed to create vertex buffer.");

    auto index = allocator.get_buffer_creator()
                     .size(10000)
                     .usage(hgn::buffer_usage_flag_bits::index)
                     .create();
    assert(index and "Failed to create index buffer.");

    auto uniform = allocator.get_buffer_creator()
                       .size(10000)
                       .usage(hgn::buffer_usage_flag_bits::uniform)
                       .create();
    assert(uniform and "Failed to create uniform buffer.");

    return buffers{
        .vertex = std::move(*vertex), .index = std::move(*index), .uniform = std::move(*uniform)
    };
}