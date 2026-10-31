#pragma once

#include <cstdint>
#include <expected>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/pipeline.hpp"
#include "hgn/raii/command.hpp"
#include "hgn/raw.hpp"

namespace hgn {
class image_memory_barrier_executor final {
    friend class command_buffer;

  public:
    explicit image_memory_barrier_executor(raw::command_buffer& buffer) noexcept
        : buffer_{std::ref(buffer)} {
        raw_.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

        raw_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        raw_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        raw_.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        raw_.subresourceRange.baseMipLevel   = 0;
        raw_.subresourceRange.levelCount     = 1;
        raw_.subresourceRange.baseArrayLayer = 0;
        raw_.subresourceRange.layerCount     = 1;
    }

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

    void execute() noexcept {
        auto source      = VkPipelineStageFlagBits{};
        auto destination = VkPipelineStageFlagBits{};
        if (raw_.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED and
            raw_.newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            raw_.srcAccessMask = 0;
            raw_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            source             = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } else if (
            raw_.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and
            raw_.newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        ) {
            raw_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            raw_.dstAccessMask = 0;
            source             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destination        = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }

        vkCmdPipelineBarrier(
            &buffer_.get(), source, destination, 0, 0, nullptr, 0, nullptr, 1, &raw_
        );
    }

  private:
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

    void execute() noexcept {
        raw_.colorAttachmentCount = 1;
        raw_.pColorAttachments    = &attachment_;

        vkCmdBeginRendering(&buffer_.get(), &raw_);
    }

  private:
    explicit command_begin_rendering_executor(raw::command_buffer& buffer) noexcept
        : buffer_{std::ref(buffer)} {
        raw_.sType             = VK_STRUCTURE_TYPE_RENDERING_INFO;
        raw_.renderArea.offset = {.x = 0, .y = 0};
        raw_.layerCount        = 1;

        attachment_.sType            = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachment_.imageLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_.clearValue.color = {{1.f, 1.f, 1.f, 1.f}};
    }

    std::reference_wrapper<raw::command_buffer> buffer_;
    VkRenderingInfo                             raw_{};
    VkRenderingAttachmentInfo                   attachment_{};
};

class command_buffer final {
    friend class command_pool;

  public:
    [[nodiscard]] auto begin_command() noexcept -> std::expected<void, result> {
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

    void bind(graphics_pipeline& pipe) noexcept {
        vkCmdBindPipeline(&get(), VK_PIPELINE_BIND_POINT_GRAPHICS, &pipe.get());
    }

    void bind(buffer& buffer) noexcept {
        const auto offset = std::uint64_t{};
        auto*      ptr    = &buffer.get();
        vkCmdBindVertexBuffers(&get(), 0, 1, &ptr, &offset);
    }

    void draw(const std::uint32_t vertex_count, const std::uint32_t instance_count) noexcept {
        vkCmdDraw(&get(), vertex_count, instance_count, 0, 1);
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

    [[nodiscard]] auto allocate_buffer() noexcept -> std::expected<command_buffer, result> {
        auto info               = VkCommandBufferAllocateInfo{};
        info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
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