#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

#include <sbwi/stb_image.h>

namespace sbwi {
class image final {
    static void delete_image(std::byte* ptr) noexcept {
        stbi_image_free(reinterpret_cast<unsigned char*>(ptr));
    }

  public:
    image(
        std::byte*          data,
        const std::uint32_t width,
        const std::uint32_t height,
        const std::uint32_t channels
    ) noexcept
        : data_{data, delete_image}, width_{width}, height_{height}, channels_{channels} {}

    [[nodiscard]] auto data() noexcept -> std::byte* { return data_.get(); }

    [[nodiscard]] auto width() const noexcept -> std::uint32_t { return width_; }

    [[nodiscard]] auto height() const noexcept -> std::uint32_t { return height_; }

    [[nodiscard]] auto channels() const noexcept -> std::uint32_t { return channels_; }

  private:
    std::unique_ptr<std::byte, void (*)(std::byte*)> data_;
    std::uint32_t                                    width_;
    std::uint32_t                                    height_;
    std::uint32_t                                    channels_;
};

[[nodiscard]] inline auto create_image(const std::filesystem::path& path
) noexcept -> std::optional<image> {
    auto  width    = 0;
    auto  height   = 0;
    auto  channels = 0;
    auto* data     = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (data == nullptr) return std::nullopt;

    return image{
        reinterpret_cast<std::byte*>(data),
        static_cast<std::uint32_t>(width),
        static_cast<std::uint32_t>(height),
        static_cast<std::uint32_t>(channels)
    };
}
}  // namespace sbwi