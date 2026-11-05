#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/raii/command.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/rendering.hpp"

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

class command_begin_rendering_executor final {
    friend class command_buffer;

  public:
    auto image_view(image_view& view) noexcept -> command_begin_rendering_executor& {
        attachment_.imageView = &view.get();
        return *this;
    }

    auto clear_color(const float r, const float g, const float b, const float a) noexcept
        -> command_begin_rendering_executor& {
        attachment_.clearValue.color = {{r, g, b, a}};
        return *this;
    }

    auto extent(const std::uint32_t width, const std::uint32_t height) noexcept
        -> command_begin_rendering_executor& {
        raw_.renderArea.extent = {.width = width, .height = height};
        return *this;
    }

    void begin() noexcept {
        raw_.colorAttachmentCount = 1;
        raw_.pColorAttachments    = &attachment_;

        vkCmdBeginRendering(&buffer_.get(), &raw_);
    }

  private:
    explicit command_begin_rendering_executor(raw::command_buffer& buffer) noexcept
        : buffer_{std::ref(buffer)} {
        raw_.sType             = VK_STRUCTURE_TYPE_RENDERING_INFO;
        raw_.renderArea.offset = {.x = 0, .y = 0};  // 画面描画範囲のオフセット
        raw_.layerCount        = 1;

        attachment_.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        // 書いている最中の画像のレイアウト
        attachment_.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // 書き始める前にそのキャンバスをどうするか。以下なら最初に指定した色で塗りつぶす
        attachment_.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // 書き終わった後にその結果をどう扱うか。以下なら次の処理のためVRAMに保存
        attachment_.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_.clearValue.color = {{1.f, 1.f, 1.f, 1.f}};
    }

    std::reference_wrapper<raw::command_buffer> buffer_;
    VkRenderingInfo                             raw_{};
    VkRenderingAttachmentInfo                   attachment_{};
};

class command_buffer final {
    friend class command_pool;

  public:
    [[nodiscard]] auto begin() noexcept -> std::expected<void, result> {
        auto info      = VkCommandBufferBeginInfo{};
        info.sType     = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags     = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        const auto res = vkBeginCommandBuffer(&get(), &info);
        if (res != VK_SUCCESS) return make_unexpected(res);

        return {};
    }

    [[nodiscard]] auto get_image_memory_barrier_executor() noexcept
        -> image_memory_barrier_executor {
        return image_memory_barrier_executor{get()};
    }

    [[nodiscard]] auto get_begin_rendering_executor() noexcept -> command_begin_rendering_executor {
        return command_begin_rendering_executor{get()};
    }

    void bind_pipeline(graphics_pipeline& pipe) noexcept {
        vkCmdBindPipeline(&get(), VK_PIPELINE_BIND_POINT_GRAPHICS, &pipe.get());
    }

    void bind_vertex_buffer(buffer& buffer) noexcept {
        const auto offset = std::uint64_t{};
        auto*      ptr    = &buffer.get();
        vkCmdBindVertexBuffers(&get(), 0, 1, &ptr, &offset);
    }

    void bind_index_buffer(buffer& buffer) noexcept {
        vkCmdBindIndexBuffer(&get(), &buffer.get(), 0, VK_INDEX_TYPE_UINT32);
    }

    void draw(const std::uint32_t vertex_count, const std::uint32_t instance_count) noexcept {
        vkCmdDraw(&get(), vertex_count, instance_count, 0, 1);
    }

    void draw_indexed(
        const std::uint32_t index_count, const std::uint32_t instance_count
    ) noexcept {
        vkCmdDrawIndexed(&get(), index_count, instance_count, 0, 0, 0);
    }

    void set_viewport(const float width, const float height) noexcept {
        auto vp     = VkViewport{};
        vp.width    = width;
        vp.height   = height;
        vp.minDepth = 0;
        vp.maxDepth = 1;

        vkCmdSetViewport(&get(), 0, 1, &vp);
    }

    void end_rendering() noexcept { vkCmdEndRendering(&get()); }

    [[nodiscard]] auto end() noexcept -> std::expected<void, result> {
        const auto res = vkEndCommandBuffer(&get());
        if (res != VK_SUCCESS) return make_unexpected(res);
        return {};
    }

    void set_scissor(const std::uint32_t width, const std::uint32_t height) noexcept {
        auto scissor   = VkRect2D{};
        scissor.offset = {.x = 0, .y = 0};
        scissor.extent = {.width = width, .height = height};

        vkCmdSetScissor(&get(), 0, 1, &scissor);
    }

    [[nodiscard]] auto get() noexcept -> raw::command_buffer& { return raw_; }

  private:
    explicit command_buffer(raw::command_buffer& raw) noexcept : raw_{std::move(raw)} {}

    raii::command_buffer raw_;
};

class command_pool final {
    friend class device;

  public:
    [[nodiscard]] auto get() noexcept -> raw::command_pool& { return raw_; }

    [[nodiscard]] auto create_buffer() noexcept -> std::expected<command_buffer, result> {
        auto info  = VkCommandBufferAllocateInfo{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

        // コマンドバッファのレベルを示す
        // これは選択可能
        // PRIMARYは直接キューにSubmitできるメイン
        // SECONDARYはプライマリから呼び出して実行するためのサブ
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        // 一度に割り当てるコマンドバッファの数
        // 複数の場合も配列として渡すことが可能
        info.commandBufferCount = 1;
        info.commandPool        = &get();

        auto*      raw = static_cast<raw::command_buffer*>(nullptr);
        const auto res = vkAllocateCommandBuffers(&raw_.init_device(), &info, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return command_buffer{*raw};
    }

  private:
    explicit command_pool(raw::command_pool& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::command_pool raw_;
};
}  // namespace hgn