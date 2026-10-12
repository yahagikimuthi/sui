#pragma once

#include <vulkan/vulkan.h>
#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"

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

    [[nodiscard]] auto native() const noexcept -> const VkCommandBufferBeginInfo& {
        return native_;
    }

  private:
    VkCommandBufferBeginInfo native_{};
};

[[nodiscard]] inline auto try_begin_command_buffer(
    command_buffer& buffer, const command_buffer_begin_info& info
) noexcept -> std::expected<void, result> {
    const auto res = vkBeginCommandBuffer(&buffer, &info.native());
    if (res != VK_SUCCESS) return make_result(res);
    return {};
}

class image_memory_barrier final {
  public:
    explicit image_memory_barrier() noexcept {
        native_.sType     = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        native_.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    [[nodiscard]] auto native() const noexcept -> const VkImageMemoryBarrier& { return native_; }

  private:
    VkImageMemoryBarrier native_{};
};

inline void command_pipeline_barrier(command_buffer& buffer, image_memory_barrier& barrier) {
    vkCmdPipelineBarrier(
        &buffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier.native()
    );
}

inline auto try_end_command_buffer(command_buffer& buffer) noexcept -> std::expected<void, result> {
    const auto res = vkEndCommandBuffer(&buffer);
    if (res != VK_SUCCESS) return make_result(res);
    return {};
}
}  // namespace hgn