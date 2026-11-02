#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/command.hpp"
#include "hgn/detail/device.hpp"
#include "hgn/detail/pipeline_creator.hpp"
#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/queue.hpp"
#include "hgn/queue_family.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/rendering.hpp"
#include "hgn/swapchain.hpp"
#include "hgn/sync.hpp"

namespace hgn {
class device final {
    friend class physical_device;

  public:
    device(const device&) noexcept                    = delete;
    auto operator=(const device&) noexcept -> device& = delete;
    device(device&&) noexcept                         = default;
    auto operator=(device&&) noexcept -> device&      = default;
    ~device() noexcept                                = default;

    [[nodiscard]] auto get_queue(const queue_family& family) noexcept -> queue {
        auto* raw = static_cast<raw::queue*>(nullptr);
        vkGetDeviceQueue(&get(), family.index(), 0, &raw);

        assert(raw != nullptr);
        return queue{*raw};
    }

    [[nodiscard]] auto get_swapchain_creator() noexcept -> swapchain_khr_creator {
        return swapchain_khr_creator{get()};
    }

    [[nodiscard]] auto create_semaphore() noexcept -> std::expected<semaphore, result> {
        auto info  = VkSemaphoreCreateInfo{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        auto*      raw = static_cast<raw::semaphore*>(nullptr);
        const auto res = vkCreateSemaphore(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return semaphore{*raw, get()};
    }

    [[nodiscard]] auto create_fence() noexcept -> std::expected<fence, result> {
        auto info  = VkFenceCreateInfo{};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        auto*      raw = static_cast<raw::fence*>(nullptr);
        const auto res = vkCreateFence(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return fence{*raw, get()};
    }

    [[nodiscard]] auto create_command_pool(const queue_family& family) noexcept
        -> std::expected<command_pool, result> {
        auto info             = VkCommandPoolCreateInfo{};
        info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = family.index();

        auto*      raw = static_cast<raw::command_pool*>(nullptr);
        const auto res = vkCreateCommandPool(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return command_pool{*raw, get()};
    }

    [[nodiscard]] auto get_image_view_creator() noexcept -> image_view_creator {
        return image_view_creator{get()};
    }

    [[nodiscard]] auto create_buffer(const std::uint64_t size) noexcept
        -> std::expected<buffer, result> {
        auto info        = VkBufferCreateInfo{};
        info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        info.size        = size;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto*      raw = static_cast<raw::buffer*>(nullptr);
        const auto res = vkCreateBuffer(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return buffer{*raw, get()};
    }

    [[nodiscard]] auto create_memory(
        const std::uint64_t allocation_size, const std::uint32_t memory_type_index
    ) noexcept -> std::expected<device_memory, result> {
        auto info            = VkMemoryAllocateInfo{};
        info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize  = allocation_size;
        info.memoryTypeIndex = memory_type_index;

        auto*      raw = static_cast<raw::device_memory*>(nullptr);
        const auto res = vkAllocateMemory(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return device_memory{*raw, get()};
    }

    [[nodiscard]] auto create_shader(const std::span<const char> codes) noexcept
        -> std::expected<shader_module, result> {
        auto info     = VkShaderModuleCreateInfo{};
        info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = codes.size();

        // TODO Configure this!
        info.pCode = reinterpret_cast<const std::uint32_t*>(codes.data());

        auto*      raw = static_cast<raw::shader_module*>(nullptr);
        const auto res = vkCreateShaderModule(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return shader_module{*raw, get()};
    }

    [[nodiscard]] auto create_pipeline_layout() noexcept -> std::expected<pipeline_layout, result> {
        auto info                   = VkPipelineLayoutCreateInfo{};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount         = 0;
        info.pushConstantRangeCount = 0;

        auto*      raw = static_cast<raw::pipeline_layout*>(nullptr);
        const auto res = vkCreatePipelineLayout(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return pipeline_layout{*raw, get()};
    }

    [[nodiscard]] auto get_graphics_pipeline_creator() noexcept -> graphics_pipeline_creator {
        return graphics_pipeline_creator{get()};
    }

    [[nodiscard]] auto wait_idle() noexcept -> std::expected<void, result> {
        const auto res = vkDeviceWaitIdle(&get());
        if (res != VK_SUCCESS) return make_unexpected(res);
        return {};
    }

    [[nodiscard]] auto get() noexcept -> raw::device& { return raw_; }

  private:
    explicit device(raw::device& raw) noexcept : raw_{std::move(raw)} {}

    raii::device raw_;
};
}  // namespace hgn