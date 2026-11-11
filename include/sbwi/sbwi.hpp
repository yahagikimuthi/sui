#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>

#include <sbwi/stb_image.h>

#include "sbwi/raii/raii.hpp"

namespace sbwi {
class image final {
  public:
    explicit image(std::byte* pixels, int width, int height, int channels) noexcept
        : raw_{pixels}, width_{width}, height_{height}, channels_{channels} {}

  private:
    raii::image raw_;
    int         width_;
    int         height_;
    int         channels_;
};

[[nodiscard]] inline auto create_image(const std::filesystem::path& path
) noexcept -> std::optional<image> {
    auto  width    = 0;
    auto  height   = 0;
    auto  channels = 0;
    auto* pixels   = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (pixels == nullptr) return std::nullopt;

    return image{reinterpret_cast<std::byte*>(pixels), width, height, channels};
}
}  // namespace sbwi