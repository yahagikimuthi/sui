#pragma once

#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class image_memory_barrier_executor final {
    friend class command_buffer;

  public:
    auto layout(const image_layout olds, const image_layout news) noexcept
        -> image_memory_barrier_executor& {
        raw_.oldLayout = static_cast<VkImageLayout>(olds);
        raw_.newLayout = static_cast<VkImageLayout>(news);
        return *this;
    }

    auto image(image& img) noexcept -> image_memory_barrier_executor& {
        raw_.image = &img.get();
        return *this;
    }

    void barrier() noexcept {
        auto source      = VkPipelineStageFlagBits{};
        auto destination = VkPipelineStageFlagBits{};

        if (raw_.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and
            raw_.newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {  // 画像初期化時の場合
            raw_.srcAccessMask = 0;  // 初期化時なのでバリアの前には何もされていない
            raw_.dstAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  // 書き込みが終わったらバリアを終了
            source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;  // 初期化時なため何も待たない
            destination =
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;  // ピクセルが出力された瞬間
        } else if (raw_.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and
                   raw_.newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {  // 書き込み終了時の場合
            raw_.srcAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;  // 書き込みが終了したらバリアを開始
            raw_.dstAccessMask = 0;  // 終わるまでは特に何もされないため
            source = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;  // ピクセルが出力された瞬間
            destination = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;  // 全コマンドが終了するまで
            // TODO BOTTOM OF PIPE BITは非推奨化されているらしいので要修正
        }

        // sourceはバリアより前に完了していなければならないパイプラインステージ
        // destinationはバリアが完了するまで待機させられるパイプラインステージ
        vkCmdPipelineBarrier(
            &buffer_.get(), source, destination, 0, 0, nullptr, 0, nullptr, 1, &raw_
        );
    }

  private:
    explicit image_memory_barrier_executor(raw::command_buffer& buffer) noexcept
        : buffer_{std::ref(buffer)} {
        raw_.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

        // キューファミリー間の所有権行こうを行う場合の移行元のインデックスを指定
        raw_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // 移行先のインデックス。なお、同じキューファミリ内で処理するならIGNOREDを指定
        raw_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // 例えば、Transferで読み込んだ画像をGraphicsに移行するときに使う

        // 画像に対し、どのミップマップレベル、どのレイヤー、どのカラー深度アスペクト
        // に対してバリアを適用するかを指定する
        raw_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // カラー深度アスペクト
        raw_.subresourceRange.baseMipLevel = 0;  // 基準ミップマップ(元画像サイズは0)
        raw_.subresourceRange.levelCount   = 1;  // ミップマップレベルの指定
        raw_.subresourceRange.baseArrayLayer = 0;  // 画像が配列なら開始インデックスを指定
        raw_.subresourceRange.layerCount = 1;  // いくつのレイヤーを対象にするかを指定
    }

    VkImageMemoryBarrier                        raw_{};
    std::reference_wrapper<raw::command_buffer> buffer_;
};
}  // namespace hgn