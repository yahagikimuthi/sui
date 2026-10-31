#pragma once

#include <cassert>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raii/image.hpp"
#include "hgn/raw.hpp"

namespace hgn {
class image final {
    friend class swapchain_khr;

  public:
    image(const image&) noexcept                    = delete;
    auto operator=(const image&) noexcept -> image& = delete;
    image(image&&) noexcept                         = default;
    auto operator=(image&&) noexcept -> image&      = default;
    ~image() noexcept                               = default;

    [[nodiscard]] auto get() noexcept -> raw::image& { return raw_; }

  private:
    explicit image(raw::image& raw) noexcept : raw_{std::move(raw)} {}

    raii::image raw_;
};

class image_view final {
    friend class image_view_builder;

  public:
    image_view(const image_view&) noexcept                    = delete;
    auto operator=(const image_view&) noexcept -> image_view& = delete;
    image_view(image_view&&) noexcept                         = default;
    auto operator=(image_view&&) noexcept -> image_view&      = default;
    ~image_view() noexcept                                    = default;

    [[nodiscard]] auto get() noexcept -> raw::image_view& { return raw_; }

  private:
    explicit image_view(raw::image_view& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::image_view raw_;
};
}  // namespace hgn