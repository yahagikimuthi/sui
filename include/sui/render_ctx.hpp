#pragma once

#include <utility>

#include "sui/images.hpp"
#include "sui/swapchain.hpp"

namespace sui {
class render_context final {
  public:
    explicit render_context(
        swapchain&& chain, image_container&& images, image_view_container&& views
    ) noexcept
        : chain_{std::move(chain)}, images_{std::move(images)}, image_views_{std::move(views)} {}

    swapchain            chain_;
    image_container      images_;
    image_view_container image_views_;
};
}  // namespace sui