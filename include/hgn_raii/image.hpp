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

namespace hgn::raii {
class image_container final {
    template <typename T>
    using ref_w = std::reference_wrapper<T>;

  public:
    [[nodiscard]] static auto try_make(hgn::swapchain_khr& chain, hgn::device& dev) noexcept
        -> std::expected<image_container, hgn::result> {
        auto res = hgn::try_get_swapchain_images_khr(dev, chain);
        if (not res) return std::unexpected{res.error()};

        return image_container{std::move(*res)};
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return natives_.size(); }

    [[nodiscard]] auto get(const std::size_t i) noexcept -> hgn::image& { return natives_[i]; }

  private:
    explicit image_container(std::vector<ref_w<hgn::image>>&& natives) noexcept
        : natives_{std::move(natives)} {}

    std::vector<ref_w<hgn::image>> natives_;
};

[[nodiscard]] inline auto try_make_images(hgn::swapchain_khr& chain, hgn::device& dev) noexcept
    -> std::expected<image_container, hgn::result> {
    return image_container::try_make(chain, dev);
}

class image_view final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::image_view_create_info& info, hgn::device& dev
    ) noexcept -> std::expected<image_view, hgn::result> {
        auto res = hgn::try_make_image_view(dev, info);
        if (not res) return std::unexpected{res.error()};
        return image_view{*res, dev};
    }

    image_view(const image_view&) noexcept                    = delete;
    auto operator=(const image_view&) noexcept -> image_view& = delete;

    image_view(image_view&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(image_view&& other) noexcept -> image_view& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;
        return *this;
    }

    ~image_view() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::image_view& { return *native_; }

    [[nodiscard]] operator hgn::image_view&() noexcept { return get(); }

  private:
    explicit image_view(hgn::image_view& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_image_view(dev_, get());
    }

    std::optional<hgn::image_view&>     native_;
    std::reference_wrapper<hgn::device> dev_;
};
}  // namespace hgn::raii