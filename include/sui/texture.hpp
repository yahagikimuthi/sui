#pragma once

#include <cstddef>
#include <expected>
#include <utility>

#include <hgn/allocator.hpp>
#include <hgn/command.hpp>
#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <sbwi/sbwi.hpp>

#include "hgn/device.hpp"
#include "sui/error.hpp"
#include "sui/extent.hpp"

namespace sui {
class texture final {
  public:
    static auto make(hgn::allocator& allocator, hgn::command_buffer& cmd, hgn::device& dev) noexcept
        -> std::expected<texture, error> {
        if (not cmd.begin()) return make_error(runtime_error, "Failed to begin command buffer.");

        auto pixel = sbwi::create_image("../assets/test.jpg");
        if (not pixel) return make_error(runtime_error, "Failed to make stb image.");

        auto image = allocator.create_image(pixel->width(), pixel->height());
        if (not image) return make_error(runtime_error, "Failed to make vk image.");

        auto staging = allocator.get_buffer_creator()
                           .usage(hgn::buffer_usage_flag_bits::transfer_src)
                           .size(100000)
                           .create();

        if (not staging) return make_error(runtime_error, "Failed to make staging buffer.");

        cmd.get_image_memory_barrier_executor()
            .image(*image)
            .layout(hgn::image_layout::undefined, hgn::image_layout::transfer_dst_optimal)
            .barrier();

        cmd.cpy_buffer_to_image(*staging, *image, pixel->width(), pixel->height());

        cmd.get_image_memory_barrier_executor()
            .image(*image)
            .layout(
                hgn::image_layout::transfer_dst_optimal, hgn::image_layout::shader_read_only_optimal
            )
            .barrier();

        if (not cmd.end()) return make_error(runtime_error, "Failed to end command buffer.");

        auto view =
            dev.get_image_view_creator().image(*image).format(hgn::format::r8g8b8a8_srgb).create();
        if (not view) return make_error(runtime_error, "Failed to make image view.");

        return texture{std::move(*image), std::move(*view), std::move(*pixel)};
    }

    [[nodiscard]] auto pixels() noexcept -> std::byte* { return image_.pixels(); }

    [[nodiscard]] auto size() noexcept -> extent {
        return {
            .width  = static_cast<float>(image_.width()),
            .height = static_cast<float>(image_.height())
        };
    }

  private:
    explicit texture(hgn::image&& raw, hgn::image_view&& view, sbwi::image&& image) noexcept
        : raw_{std::move(raw)}, view_{std::move(view)}, image_{std::move(image)} {}

    hgn::image      raw_;
    hgn::image_view view_;
    sbwi::image     image_;
};
}  // namespace sui