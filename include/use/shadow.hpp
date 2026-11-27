#pragma once

#include <array>
#include <cassert>

#include <hgn/allocator.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/physical.hpp>

#include "use/structs.hpp"

[[nodiscard]] inline auto create_shadow(
    hgn::physical_device& physical, hgn::device& dev, hgn::allocator& allocator
) noexcept -> shadow {
    auto aligned_formats =
        std::array{
            hgn::format::d32_sfloat, hgn::format::d24_unorm_s8_uint, hgn::format::d32_sfloat_s8_uint
        } |
        std::views::filter([&](hgn::format fmt) noexcept -> bool {
            return physical.is_supporting_depth_format(fmt);
        });

    assert(not aligned_formats.empty());

    const auto depth_fmt = aligned_formats.front();
}