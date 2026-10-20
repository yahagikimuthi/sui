#pragma once

#include <cstddef>
#include <expected>
#include <functional>
#include <utility>
#include <vector>

#include <hgn/device.hpp>
#include <hgn/image.hpp>
#include <hgn/swapchain.hpp>

#include "hgn/enums.hpp"
#include "sui/error.hpp"

namespace sui {
class image_container final {
    template <typename T>
    using ref_w = std::reference_wrapper<T>;

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

    std::vector<ref_w<hgn::image>> images_;
};

class image_view_container final {
    template <typename T>
    using ref_w = std::reference_wrapper<T>;

  public:
    [[nodiscard]] static auto try_make(
        image_container& images, hgn::device& dev, const hgn::format format
    ) noexcept -> std::expected<image_view_container, error> {
        auto views = std::vector<ref_w<hgn::image_view>>{};
        views.reserve(images.size());

        for (const auto i : std::views::indices(images.size())) {
            auto info = hgn::image_view_create_info{};
            info.setting_image(images[i]).view_type(hgn::image_view_type::type_2d).format(format);

            auto res = hgn::try_make_image_view(dev, info);
            if (not res) return make_error(runtime_error, "Failed to make swapchain image view.");
            views.emplace_back(std::ref(*res));
        }

        return image_view_container{std::move(views), dev};
    }

    image_view_container(const image_view_container&) noexcept = delete;

    auto operator=(const image_view_container&) noexcept -> image_view_container& = delete;

    image_view_container(image_view_container&& other) noexcept = default;

    auto operator=(image_view_container&& other) noexcept -> image_view_container& = default;

    ~image_view_container() noexcept { destroy(); }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return views_.size(); }

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> hgn::image_view& {
        return views_[i].get();
    }

    explicit image_view_container(
        std::vector<ref_w<hgn::image_view>>&& views, hgn::device& dev
    ) noexcept
        : views_{std::move(views)}, dev_{std::ref(dev)} {}

  private:
    void destroy() noexcept {
        for (auto view : views_) hgn::destroy_image_view(dev_.get(), view.get());
    }

    std::vector<ref_w<hgn::image_view>> views_;
    ref_w<hgn::device>                  dev_;
};
}  // namespace sui