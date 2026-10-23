#pragma once

#include "vulkan/vulkan_core.h"

#include "hgn/command.hpp"
#include "hgn/image.hpp"

namespace hgn {
class rendering_attachment_info final {
  public:
    explicit rendering_attachment_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;

        native_.imageLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        native_.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        native_.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        native_.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        native_.clearValue.color = {{1.f, 1.f, 1.f, 1.f}};
    }

    auto setting_image_view(image_view& view) noexcept -> rendering_attachment_info& {
        native_.imageView = &view;
        return *this;
    }

    auto clear_color(const float r, const float g, const float b, const float a) noexcept
        -> rendering_attachment_info& {
        native_.clearValue.color = {{r, g, b, a}};
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkRenderingAttachmentInfo& { return native_; }

  private:
    VkRenderingAttachmentInfo native_{};
};

class rendering_info final {
  public:
    explicit rendering_info() noexcept {
        native_.sType             = VK_STRUCTURE_TYPE_RENDERING_INFO;
        native_.renderArea.offset = {.x = 0, .y = 0};
        native_.layerCount        = 1;
    }

    auto extent(const std::uint32_t x, const std::uint32_t y) noexcept -> rendering_info& {
        native_.renderArea.extent = {.width = x, .height = y};
        return *this;
    }

    auto color_attachment(const rendering_attachment_info& info) noexcept -> rendering_info& {
        native_.colorAttachmentCount = 1;
        native_.pColorAttachments    = &info.get();
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkRenderingInfo& { return native_; }

  private:
    VkRenderingInfo native_{};
};

inline void command_begin_rendering(command_buffer& cmd_buf, const rendering_info& info) noexcept {
    vkCmdBeginRendering(&cmd_buf, &info.get());
}

inline void command_end_rendering(command_buffer& cmd_buf) noexcept {
    vkCmdEndRendering(&cmd_buf);
}
}  // namespace hgn