#pragma once

#include <bit>
#include <cstdint>
#include <functional>
#include <ranges>
#include <span>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/queue_family.hpp"
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

class image_view_creator final {
    friend class device;

  public:
    // 元となる画像
    auto image(image& image) noexcept -> image_view_creator& {
        info_.image = &image.get();
        return *this;
    }

    // ビット解釈の設定
    auto format(const format fmt) noexcept -> image_view_creator& {
        info_.format = static_cast<VkFormat>(fmt);
        return *this;
    }

    [[nodiscard]] auto create() const noexcept -> std::expected<image_view, result> {
        auto*      raw = static_cast<raw::image_view*>(nullptr);
        const auto res = vkCreateImageView(&dev_.get(), &info_, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return image_view{*raw, dev_.get()};
    }

  private:
    explicit image_view_creator(raw::device& dev) noexcept : dev_{std::ref(dev)} {
        info_.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        // 画像のRGBAの順を入れ替えたり固定値に設定するための機能
        // これは同じ画像をそのまま描画する
        // 常に赤チャンネルを1に固定するなどもできる
        info_.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        info_.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        info_.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        info_.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // 以下は元画像のどの領域、どの層を対象とするか範囲指定する
        // どの種別にアクセスするか。カラーならこれ、深度の場合も設定値がある
        info_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info_.subresourceRange.baseMipLevel = 0;  // ミップマップレベル(0が最も解像度高)
        info_.subresourceRange.levelCount = 1;      // いくつのレベルを含めるか
        info_.subresourceRange.baseArrayLayer = 0;  // アクセスを開始する層番号(2Dなら0)
        info_.subresourceRange.layerCount = 1;  // いくつのレイヤーを含めるか(Cubeなら8)

        // 補足
        // 1.ミップマップ
        // 端的に言えば画像の縮尺のこと
        // レベル0が元サイズ, 1で半分(面積は1/4), 2で更に半分になる。これが(1,1)ピクセルまで続く
        // 遠くのオブジェクトには小さい画像を使うことで、解像度が小さくすることが可能
        // また、巨大なメモリを消費せず、小さい画像として扱うこともできるため高速になる

        // 2.レイヤー
        // 同じ大きさの画像を何枚も重ねてVkImageとして扱うための仕組み
        // パラパラマンガのように、同じサイズの画像が層状に積み重なっているイメージ
        // キューブマップに使ったり、配列のように複数レイヤーのうち1つだけを使うなども可能になる

        // 画像解釈の設定
        // 他にも1D, 3D, キューブなどがある
        info_.viewType = VK_IMAGE_VIEW_TYPE_2D;

        info_.pNext = nullptr;  // 通常はnullptr
        info_.flags = 0;  // アトミック操作など特別な動作を有効にする場合は使用
    }

    VkImageViewCreateInfo               info_{};
    std::reference_wrapper<raw::device> dev_;
};
}  // namespace hgn