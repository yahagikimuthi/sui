#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include <hgn/buffer.hpp>
#include <hgn/image.hpp>

namespace wk {
enum class image : std::size_t {};
enum class image_view : std::size_t {};
enum class buffer : std::size_t {};
}  // namespace wk

namespace wk::detail {
class resource_manager final {
  public:
    [[nodiscard]] auto push_image(hgn::image&& img) noexcept -> image {
        const auto idx = images_.size();
        images_.emplace_back(std::move(img));
        return static_cast<image>(idx);
    }

    [[nodiscard]] auto push_view(hgn::image_view&& view) noexcept -> image_view {
        const auto idx = views_.size();
        views_.emplace_back(std::move(view));
        return static_cast<image_view>(idx);
    }

    [[nodiscard]] auto push_buffer(hgn::buffer&& b) noexcept -> buffer {
        const auto idx = buffers_.size();
        buffers_.emplace_back(std::move(b));
        return static_cast<buffer>(idx);
    }

  private:
    std::vector<hgn::image>      images_;
    std::vector<hgn::image_view> views_;
    std::vector<hgn::buffer>     buffers_;
};
}  // namespace wk::detail