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
    auto vertex = *allocator.get_buffer_creator()
                       .size(10000)
                       .usage(hgn::buffer_usage_flag_bits::vertex)
                       .create();

    auto index = *allocator.get_buffer_creator()
                      .size(10000)
                      .usage(hgn::buffer_usage_flag_bits::index)
                      .create();

    auto uniform = *allocator.get_buffer_creator()
                        .size(10000)
                        .usage(hgn::buffer_usage_flag_bits::uniform)
                        .create();

    return buffers{
        .vertex = std::move(vertex), .index = std::move(index), .uniform = std::move(uniform)
    };
}