#pragma once

#include <expected>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class image_view_creator final {
  public:
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
        auto* raw = static_cast<raw::image_view*>(nullptr);
        return make_expected(vkCreateImageView(&dev_.get(), &info_, nullptr, &raw))
            .transform([&]() noexcept -> image_view { return image_view{*raw, dev_.get()}; });
    }

  private:
    VkImageViewCreateInfo               info_{};
    std::reference_wrapper<raw::device> dev_;
};
}  // namespace hgn