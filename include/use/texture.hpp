#pragma once

#include <cassert>

#include <hgn/allocator.hpp>
#include <hgn/command.hpp>

#include "hgn/buffer.hpp"
#include "hgn/descriptor.hpp"
#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/queue.hpp"
#include "sbwi/sbwi.hpp"
#include "use/structs.hpp"

[[nodiscard]] inline auto create_texture(
    hgn::allocator&             allocator,
    hgn::device&                dev,
    hgn::command_pool&          cmd_pool,
    hgn::queue&                 graphics_queue,
    hgn::descriptor_set_layout& layout,
    hgn::descriptor_pool&       pool,
    hgn::buffer&                uniform
) noexcept -> texture {
    auto pixel = *sbwi::create_image("../assets/test.jpg");

    auto img =
        *allocator.get_image_creator()
             .extent(pixel.width(), pixel.height())
             .format(hgn::format::r8g8b8a8_srgb)
             .usage(hgn::image_usage_flag_bits::transfer_dst | hgn::image_usage_flag_bits::sampled)
             .create();

    auto buffer = *allocator.get_buffer_creator()
                       .size(1000000)
                       .usage(hgn::buffer_usage_flag_bits::transfer_src)
                       .create();

    buffer.cpy(pixel.data(), pixel.width() * pixel.height() * 4);

    auto cmd = *cmd_pool.create_buffer();
    *cmd.begin();

    cmd.get_image_memory_barrier_executor()
        .image(img)
        .layout(hgn::image_layout::undefined, hgn::image_layout::transfer_dst_optimal)
        .aspect_mask(hgn::image_aspect_flag_bits::color)
        .barrier();

    cmd.cpy_buffer_to_image(buffer, img, pixel.width(), pixel.height());

    cmd.get_image_memory_barrier_executor()
        .image(img)
        .layout(
            hgn::image_layout::transfer_dst_optimal, hgn::image_layout::shader_read_only_optimal
        )
        .aspect_mask(hgn::image_aspect_flag_bits::color)
        .barrier();

    *cmd.end();
    auto view = *dev.get_image_view_creator()
                     .image(img)
                     .aspect_mask(hgn::image_aspect_flag_bits::color)
                     .format(hgn::format::r8g8b8a8_srgb)
                     .create();

    auto sampler = *dev.create_sampler();

    auto set = *pool.get_descriptor_set(layout);

    set.get_writer()
        .get_pusher()
        .binding(0)
        .type(hgn::descriptor_type::uniform_buffer)
        .buffer(uniform)
        .push()
        .get_pusher()
        .binding(1)
        .image(view, sampler)
        .type(hgn::descriptor_type::combined_image_sampler)
        .push()
        .write();

    *graphics_queue.get_submitter().command_buffer(cmd).submit();
    *graphics_queue.wait_idle();

    return {
        .pixel   = std::move(pixel),
        .image   = std::move(img),
        .view    = std::move(view),
        .sampler = std::move(sampler),
        .set     = set
    };
}