#pragma once

#include <functional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raii/image.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class image final {
  public:
    explicit image(raw::image& raw, raw::allocator& allocator, raw::allocation& alloc) noexcept
        : raw_{std::move(raw), allocator, alloc} {}

    [[nodiscard]] auto get() noexcept -> raw::image& { return raw_; }

  private:
    raii::image raw_;
};

class borrowed_image final {
  public:
    explicit borrowed_image(raw::image& raw) noexcept : raw_{std::ref(raw)} {}

    borrowed_image(image& img) noexcept : raw_{std::ref(img.get())} {}

    [[nodiscard]] auto get() noexcept -> raw::image& { return raw_; }

  private:
    std::reference_wrapper<raw::image> raw_;
};

class image_view final {
  public:
    explicit image_view(raw::image_view& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    [[nodiscard]] auto get() noexcept -> raw::image_view& { return raw_; }

  private:
    raii::image_view raw_;
};
}  // namespace hgn