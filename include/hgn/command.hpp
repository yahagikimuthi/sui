#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <iostream>

#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/image.hpp"

namespace hgn {
using command_pool   = VkCommandPool_T;
using command_buffer = VkCommandBuffer_T;

class command_pool_create_info final {
  public:
    explicit command_pool_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        native_.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }

    auto queue_family_index(const std::uint32_t index) noexcept -> command_pool_create_info& {
        native_.queueFamilyIndex = index;
        return *this;
    }

  private:
    VkCommandPoolCreateInfo native_{};
};

[[nodiscard]] inline auto try_make_command_pool(
    device& dev, command_pool_create_info& info
) noexcept -> std::expected<std::reference_wrapper<command_pool>, result> {
    auto* pool        = static_cast<command_pool*>(nullptr);
    auto  native_info = std::bit_cast<VkCommandPoolCreateInfo>(info);

    auto res = vkCreateCommandPool(&dev, &native_info, nullptr, &pool);
    if (res != VK_SUCCESS) return make_result(res);

    assert(pool != nullptr);
    return std::ref(*pool);
}

inline void destroy_command_pool(device& dev, command_pool& target) noexcept {
    vkDestroyCommandPool(&dev, &target, nullptr);
}

class command_buffer_allocate_info final {
  public:
    explicit command_buffer_allocate_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    }

    auto level(const command_buffer_level lev) noexcept -> command_buffer_allocate_info& {
        native_.level = static_cast<VkCommandBufferLevel>(lev);
        return *this;
    }

    auto command_pool(command_pool& pool) noexcept -> command_buffer_allocate_info& {
        native_.commandPool = &pool;
        return *this;
    }

    auto command_buffer_count(const std::uint32_t count) noexcept -> command_buffer_allocate_info& {
        native_.commandBufferCount = count;
        return *this;
    }

  private:
    VkCommandBufferAllocateInfo native_{};
};

[[nodiscard]] inline auto try_allocate_command_buffers(
    device& dev, const command_buffer_allocate_info& info
) noexcept -> std::expected<std::reference_wrapper<command_buffer>, result> {
    auto* buffers     = static_cast<command_buffer*>(nullptr);
    auto  native_info = std::bit_cast<VkCommandBufferAllocateInfo>(info);

    auto res = vkAllocateCommandBuffers(&dev, &native_info, &buffers);
    if (res != VK_SUCCESS) return make_result(res);

    assert(buffers != nullptr);
    return std::ref(*buffers);
}

class command_buffer_begin_info final {
  public:
    explicit command_buffer_begin_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        native_.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    [[nodiscard]] auto get() const noexcept -> const VkCommandBufferBeginInfo& { return native_; }

  private:
    VkCommandBufferBeginInfo native_{};
};

[[nodiscard]] inline auto try_begin_command_buffer(
    command_buffer& cmd_buf, const command_buffer_begin_info& info
) noexcept -> std::expected<void, result> {
    const auto res = vkBeginCommandBuffer(&cmd_buf, &info.get());
    if (res != VK_SUCCESS) return make_result(res);
    return {};
}

class image_memory_barrier final {
  public:
    explicit image_memory_barrier() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

        native_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        native_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        native_.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        native_.subresourceRange.baseMipLevel   = 0;
        native_.subresourceRange.levelCount     = 1;
        native_.subresourceRange.baseArrayLayer = 0;
        native_.subresourceRange.layerCount     = 1;
    }

    auto layout(const image_layout old_layout, const image_layout new_layout) noexcept
        -> image_memory_barrier& {
        native_.oldLayout = static_cast<VkImageLayout>(old_layout);
        native_.newLayout = static_cast<VkImageLayout>(new_layout);

        if (old_layout == image_layout::undefined and
            new_layout == image_layout::color_attachment_optimal) {
            native_.srcAccessMask = 0;
            native_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            source_               = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_          = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            return *this;
        }
        if (old_layout == image_layout::color_attachment_optimal and
            new_layout == image_layout::present_src_khr) {
            native_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            native_.dstAccessMask = 0;
            source_               = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destination_          = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            return *this;
        }

        std::cerr << "We don't expect this pattern. please setting using native method\n";
        return *this;
    }

    auto setting_image(image& image_ref) noexcept -> image_memory_barrier& {
        native_.image = &image_ref;
        return *this;
    }

    [[nodiscard]] auto source_state() const noexcept -> VkPipelineStageFlagBits { return source_; }

    [[nodiscard]] auto destination_state() const noexcept -> VkPipelineStageFlagBits {
        return destination_;
    }

    [[nodiscard]] auto get() const noexcept -> const VkImageMemoryBarrier& { return native_; }

  private:
    VkImageMemoryBarrier    native_{};
    VkPipelineStageFlagBits source_{};
    VkPipelineStageFlagBits destination_{};
};

inline void command_pipeline_barrier(
    command_buffer& cmd_buf, image_memory_barrier& barrier
) noexcept {
    vkCmdPipelineBarrier(
        &cmd_buf,
        barrier.source_state(),
        barrier.destination_state(),
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier.get()
    );
}

[[nodiscard]] inline auto try_end_command_buffer(command_buffer& cmd_buf) noexcept
    -> std::expected<void, result> {
    const auto res = vkEndCommandBuffer(&cmd_buf);
    if (res != VK_SUCCESS) return make_result(res);
    return {};
}

inline void command_bind_vertex_buffers(command_buffer& cmd_buf, buffer& buf) noexcept {
    constexpr auto offsets = std::array<std::uint64_t, 1>{0};
    auto*          buf_ptr = &buf;
    vkCmdBindVertexBuffers(&cmd_buf, 0, 1, &buf_ptr, offsets.data());
}

inline void command_draw(
    command_buffer& cmd_buf, const std::uint32_t vertex_count, const std::uint32_t instance_count
) noexcept {
    vkCmdDraw(&cmd_buf, vertex_count, instance_count, 0, 0);
}

class viewport final {
  public:
    auto x(const float value) noexcept -> viewport& {
        native_.x = value;
        return *this;
    }

    auto y(const float value) noexcept -> viewport& {
        native_.y = value;
        return *this;
    }

    auto width(const float value) noexcept -> viewport& {
        native_.width = value;
        return *this;
    }

    auto height(const float value) noexcept -> viewport& {
        native_.height = value;
        return *this;
    }

    auto min_depth(const float value) noexcept -> viewport& {
        native_.minDepth = value;
        return *this;
    }

    auto max_depth(const float value) noexcept -> viewport& {
        native_.maxDepth = value;
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkViewport& { return native_; }

  private:
    VkViewport native_{};
};

inline void command_set_viewport(command_buffer& buf, const viewport& view) noexcept {
    vkCmdSetViewport(&buf, 0, 1, &view.get());
}
}  // namespace hgn