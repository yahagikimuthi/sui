#pragma once

#include <cstddef>
#include <type_traits>
#include <vector>

#include <hgn/buffer.hpp>
#include <hgn/image.hpp>

namespace sui {
enum class image : std::size_t {};
enum class image_view : std::size_t {};
enum class buffer : std::size_t {};

template <typename T>
    requires std::is_scoped_enum_v<T>
[[nodiscard]] constexpr auto to_index(const T e) noexcept -> std::size_t {
    return static_cast<std::size_t>(e);
}

class resource_manager final {
  public:
    resource_manager() noexcept = default;

    [[nodiscard]] auto push_image(hgn::image&& img) noexcept -> image {
        images_.emplace_back(std::move(img));
        return static_cast<image>(images_.size() - 1);
    }

    [[nodiscard]] auto get_image(image e) noexcept -> hgn::image& { return images_[to_index(e)]; }

    [[nodiscard]] auto push_image_view(hgn::image_view&& view) noexcept -> image_view {
        views_.emplace_back(std::move(view));
        return static_cast<image_view>(views_.size() - 1);
    }

    [[nodiscard]] auto get_image_view(image_view e) noexcept -> hgn::image_view& {
        return views_[to_index(e)];
    }

    [[nodiscard]] auto push_buffer(hgn::buffer&& b) noexcept -> buffer {
        buffers_.emplace_back(std::move(b));
        return static_cast<buffer>(buffers_.size() - 1);
    }

  private:
    std::vector<hgn::image>      images_;
    std::vector<hgn::image_view> views_;
    std::vector<hgn::buffer>     buffers_;
};
}  // namespace sui