#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>

#include <sbwi/stb_image.h>

#include "sbwi/raii/raii.hpp"

namespace sbwi {
class image final {
  public:
    explicit image(
        std::byte*          pixels,
        const std::uint32_t width,
        const std::uint32_t height,
        const std::uint32_t channels
    ) noexcept
        : raw_{pixels}, width_{width}, height_{height}, channels_{channels} {}

    [[nodiscard]] auto pixels() noexcept -> std::byte* { return raw_; }

    [[nodiscard]] auto width() const noexcept -> std::uint32_t { return width_; }

    [[nodiscard]] auto height() const noexcept -> std::uint32_t { return height_; }

    [[nodiscard]] auto channels() const noexcept -> std::uint32_t { return channels_; }

  private:
    raii::image   raw_;
    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t channels_;
};

[[nodiscard]] inline auto create_image(const std::filesystem::path& path
) noexcept -> std::optional<image> {
    auto  width    = 0;
    auto  height   = 0;
    auto  channels = 0;
    auto* pixels   = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (pixels == nullptr) return std::nullopt;

    return image{
        reinterpret_cast<std::byte*>(pixels),
        static_cast<std::uint32_t>(width),
        static_cast<std::uint32_t>(height),
        static_cast<std::uint32_t>(channels)
    };
}
}  // namespace sbwi