#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/allocator.hpp>
#include <hgn/buffer.hpp>
#include <hgn/enums.hpp>

namespace sui {
class buffer_container final {
  public:
    [[nodiscard]] static auto create(hgn::allocator& allocator
    ) noexcept -> std::expected<buffer_container, akr::error> {
        auto vertex = allocator.get_buffer_creator()
                          .size(10000)
                          .usage(hgn::buffer_usage_flag_bits::vertex)
                          .create();
        if (not vertex)
            return akr::make_error(akr::runtime_error, "Failed to create vertex buffer.");

        auto index = allocator.get_buffer_creator()
                         .size(10000)
                         .usage(hgn::buffer_usage_flag_bits::index)
                         .create();
        if (not index) return akr::make_error(akr::runtime_error, "Failed to create index buffer.");

        auto uniform = allocator.get_buffer_creator()
                           .size(10000)
                           .usage(hgn::buffer_usage_flag_bits::uniform)
                           .create();
        if (not uniform)
            return akr::make_error(akr::runtime_error, "Failed to create uniform buffer.");

        return buffer_container{std::move(*vertex), std::move(*index), std::move(*uniform)};
    }

  private:
    buffer_container(hgn::buffer vertex, hgn::buffer fragment, hgn::buffer uniform) noexcept
        : vertex_{std::move(vertex)},
          fragment_{std::move(fragment)},
          uniform_{std::move(uniform)} {}

    hgn::buffer vertex_;
    hgn::buffer fragment_;
    hgn::buffer uniform_;
};
}  // namespace sui