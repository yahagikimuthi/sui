#pragma once

#include <cstddef>
#include <utility>

#include <sbwi/stb_image.h>

namespace sbwi::raii {
class image final {
  public:
    explicit image(std::byte* pixels) noexcept : pixels_{reinterpret_cast<stbi_uc*>(pixels)} {}

    image(const image&) noexcept                    = delete;
    auto operator=(const image&) noexcept -> image& = delete;

    image(image&& other) noexcept : pixels_{std::exchange(other.pixels_, nullptr)} {}

    auto operator=(image&& other) noexcept -> image& {
        if (this == &other) return *this;

        destroy();

        pixels_ = std::exchange(other.pixels_, nullptr);
        return *this;
    }

    ~image() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> std::byte* {
        return reinterpret_cast<std::byte*>(pixels_);
    }

  private:
    void destroy() noexcept {
        if (pixels_ != nullptr) stbi_image_free(pixels_);
    }

    stbi_uc* pixels_;
};
}  // namespace sbwi::raii