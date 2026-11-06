#pragma once

#include <cstdint>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/image.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
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
}  // namespace hgn