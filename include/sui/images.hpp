#pragma once

#include <cstddef>
#include <expected>
#include <functional>
#include <vector>

#include <hgn/device.hpp>
#include <hgn/image.hpp>
#include <hgn/swapchain.hpp>

#include "sui/error.hpp"

namespace sui {
class image_container final {
  public:
    [[nodiscard]] static auto try_make(hgn::device& dev, hgn::swapchain& chain) noexcept
        -> std::expected<image_container, error> {
        auto res = hgn::try_get_swapchain_images_khr(dev, chain);
        if (not res) return make_error(runtime_error, "Failed to make swapchain images");

        return image_container{std::move(*res)};
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return images_.size(); }

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> hgn::image& {
        return images_[i].get();
    }

  private:
    explicit image_container(std::vector<std::reference_wrapper<hgn::image>>&& images) noexcept
        : images_{std::move(images)} {}

    std::vector<std::reference_wrapper<hgn::image>> images_;
};
}  // namespace sui