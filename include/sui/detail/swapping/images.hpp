#pragma once

#include <cstddef>
#include <expected>
#include <functional>
#include <utility>
#include <vector>

#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <hgn/swapchain.hpp>
#include <hgn_raii/image.hpp>

#include "sui/detail/hardware/device.hpp"
#include "sui/detail/swapping/swapchain.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class image_container final {
    template <typename T>
    using ref_w = std::reference_wrapper<T>;

  public:
    [[nodiscard]] static auto try_make(hgn::device& dev, swapchain& chain) noexcept
        -> std::expected<image_container, error> {
        auto res = chain.try_get_swapchain_images(dev);
        if (not res) return make_error(runtime_error, "Failed to make swapchain images");

        return image_container{std::move(*res)};
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return images_.size(); }

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> hgn::image& {
        return images_[i].get();
    }

  private:
    explicit image_container(std::vector<ref_w<hgn::image>>&& images) noexcept
        : images_{std::move(images)} {}

    std::vector<ref_w<hgn::image>> images_;
};

class image_view_container final {
    template <typename T>
    using ref_w = std::reference_wrapper<T>;

  public:
    [[nodiscard]] static auto try_make(
        image_container& images, device& dev, const hgn::format format
    ) noexcept -> std::expected<image_view_container, error> {
        auto views = std::vector<hgn::raii::image_view>{};
        views.reserve(images.size());

        for (const auto i : std::views::indices(images.size())) {
            const auto info = hgn::image_view_create_info{}.setting_image(images[i]).format(format);

            auto res = dev.try_make_image_view(info);
            if (not res) return make_error(runtime_error, "Failed to make swapchain image view.");
            views.emplace_back(std::move(*res));
        }

        return image_view_container{std::move(views)};
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return views_.size(); }

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> hgn::raii::image_view& {
        return views_[i];
    }

    explicit image_view_container(std::vector<hgn::raii::image_view>&& views) noexcept
        : views_{std::move(views)} {}

  private:
    std::vector<hgn::raii::image_view> views_;
};
}  // namespace sui::detail