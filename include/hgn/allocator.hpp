#pragma once

#include <cstdint>
#include <expected>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/detail/allocator.hpp"
#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/raii/command.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class allocator final {
  public:
    explicit allocator(raw::allocator& raw) noexcept : raw_{std::move(raw)} {}

    [[nodiscard]] auto get_buffer_creator() noexcept -> buffer_creator {
        return buffer_creator{get()};
    }

    [[nodiscard]] auto create_image(const std::uint32_t width, const std::uint32_t height) noexcept
        -> std::expected<image, result> {
        auto info          = VkImageCreateInfo{};  // NOLINT
        info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType     = VK_IMAGE_TYPE_2D;
        info.extent.width  = static_cast<uint32_t>(width);
        info.extent.height = static_cast<uint32_t>(height);
        info.extent.depth  = 1;
        info.mipLevels     = 1;
        info.arrayLayers   = 1;
        info.format        = VK_FORMAT_R8G8B8A8_SRGB;
        info.tiling        = VK_IMAGE_TILING_OPTIMAL;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        info.samples       = VK_SAMPLE_COUNT_1_BIT;

        auto imageAllocInfo  = VmaAllocationCreateInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        auto* allocation = static_cast<raw::allocation*>(nullptr);
        auto* raw        = static_cast<raw::image*>(nullptr);

        return make_expected(
                   vmaCreateImage(&get(), &info, &imageAllocInfo, &raw, &allocation, nullptr)
        )
            .transform([&]() noexcept -> image { return image{*raw, get(), *allocation}; });
    }

    [[nodiscard]] auto get() noexcept -> raw::allocator& { return raw_; }

  private:
    raii::allocator raw_;
};
}  // namespace hgn