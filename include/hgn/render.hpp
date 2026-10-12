#pragma once

#include "vulkan/vulkan_core.h"

#include "hgn/command.hpp"

namespace hgn {
class rendering_attachment_info final {
  public:
    explicit rendering_attachment_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    }

    [[nodiscard]] auto native() const noexcept -> const VkRenderingAttachmentInfo& {
        return native_;
    }

  private:
    VkRenderingAttachmentInfo native_{};
};

class rendering_info final {
  public:
    explicit rendering_info() noexcept { native_.sType = VK_STRUCTURE_TYPE_RENDERING_INFO; }

    [[nodiscard]] auto native() const noexcept -> const VkRenderingInfo& { return native_; }

  private:
    VkRenderingInfo native_{};
};

inline void command_begin_rendering(command_buffer& buffer, rendering_info& info) noexcept {
    vkCmdBeginRendering(&buffer, &info.native());
}

inline void command_end_rendering(command_buffer& buffer) noexcept {
    vkCmdEndRendering(&buffer);
}
}  // namespace hgn