#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <ranges>
#include <span>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/queue.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/structs.hpp"
#include "hgn/surface.hpp"
#include "hgn/swapchain.hpp"

namespace hgn {
class swapchain_khr_creator final {
    friend class device;

  public:
    auto surface(surface_khr& surface) noexcept -> swapchain_khr_creator& {
        info_.surface = &surface.get();
        return *this;
    }

    // swapchainが保持する最小画像枚数の設定
    // 実際の枚数はこれ以上になる場合がある
    // そのswapchainが保持できる枚数以上の値が入力されることを期待する
    auto min_image_count(const std::uint32_t count) noexcept -> swapchain_khr_creator& {
        info_.minImageCount = count;
        return *this;
    }

    // ピクセルフォーマット(ビット解釈)の設定
    auto image_format(const format fmt) noexcept -> swapchain_khr_creator& {
        info_.imageFormat = static_cast<VkFormat>(fmt);
        return *this;
    }

    // 色空間の設定
    auto image_color_space(const color_space_khr space) noexcept -> swapchain_khr_creator& {
        info_.imageColorSpace = static_cast<VkColorSpaceKHR>(space);
        return *this;
    }

    // 画像の解像度(幅と高さ)の設定
    auto image_extent(const extent2d extent) noexcept -> swapchain_khr_creator& {
        info_.imageExtent = std::bit_cast<VkExtent2D>(extent);
        return *this;
    }

    // 画面の回転を設定
    auto pre_transform(const surface_transform_flag_bits_khr transform
    ) noexcept -> swapchain_khr_creator& {
        info_.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(transform);
        return *this;
    }

    // 描写/垂直同期の挙動設定
    auto present_mode(const present_mode_khr mode) noexcept -> swapchain_khr_creator& {
        info_.presentMode = static_cast<VkPresentModeKHR>(mode);
        return *this;
    }

    // キューファミリーの設定
    auto queue_families(const std::span<const queue_family> families
    ) noexcept -> swapchain_khr_creator& {
        queue_family_indices_.assign_range(families | std::views::transform(&queue_family::index));
        return *this;
    }

    // 共有モードの設定
    auto image_sharing_mode(const sharing_mode mode) noexcept -> swapchain_khr_creator& {
        info_.imageSharingMode = static_cast<VkSharingMode>(mode);
        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::expected<swapchain_khr, result> {
        info_.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_family_indices_.size());
        info_.pQueueFamilyIndices   = queue_family_indices_.data();

        auto*      raw = static_cast<raw::swapchain_khr*>(nullptr);
        const auto res = vkCreateSwapchainKHR(&dev_.get(), &info_, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return swapchain_khr{*raw, dev_.get(), static_cast<format>(info_.imageFormat)};
    }

  private:
    explicit swapchain_khr_creator(raw::device& dev) noexcept : dev_{std::ref(dev)} {
        info_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        // 描画先としてメモリを確保する場合は以下の定数を設定
        // 他にもコピー先/コピー元として扱う、読み書き用などがある
        info_.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        info_.imageArrayLayers = 1;  // 各画像の層数. 立体視やVRを除き通常は1

        // アルファチャンネル(通常は不透明値のこの定数を設定)
        info_.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        info_.clipped = VK_TRUE;  // ウィンドウに隠れたピクセルを省略するかを設定
        info_.oldSwapchain = nullptr;  // リサイズ時などに古いswapchainを破棄, 再利用するため設定
    }

    VkSwapchainCreateInfoKHR            info_{};  // NOLINT
    std::vector<std::uint32_t>          queue_family_indices_;
    std::reference_wrapper<raw::device> dev_;
};
}  // namespace hgn