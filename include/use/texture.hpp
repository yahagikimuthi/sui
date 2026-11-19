#pragma once

#include <cassert>

#include <hgn/allocator.hpp>
#include <hgn/command.hpp>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/queue.hpp"
#include "sbwi/sbwi.hpp"
#include "use/structs.hpp"

[[nodiscard]] inline auto create_texture(
    hgn::allocator& allocator, hgn::device& dev, hgn::queue_family graphics
) noexcept -> texture {
    auto pixel = *sbwi::create_image("../assets/test.jpg");

    auto img =
        *allocator.get_image_creator()
             .extent(pixel.width(), pixel.height())
             .format(hgn::format::r8g8b8a8_srgb)
             .usage(hgn::image_usage_flag_bits::transfer_dst | hgn::image_usage_flag_bits::sampled)
             .create();

    auto view = *dev.get_image_view_creator()
                     .image(img)
                     .aspect_mask(hgn::image_aspect_flag_bits::color)
                     .format(hgn::format::r8g8b8a8_srgb)
                     .create();

    auto buffer = *allocator.get_buffer_creator()
                       .size(100000)
                       .usage(hgn::buffer_usage_flag_bits::transfer_src)
                       .create();

    auto cmd_pool = *dev.create_command_pool(graphics);
    auto cmd      = *cmd_pool.create_buffer();
    assert(cmd.begin());

    assert(cmd.end());
}