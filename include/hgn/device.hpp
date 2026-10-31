#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/command.hpp"
#include "hgn/device_queue.hpp"
#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/physical.hpp"
#include "hgn/pipeline_layout.hpp"
#include "hgn/pipeline_structs.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/shader.hpp"
#include "hgn/structs.hpp"
#include "hgn/sync.hpp"

namespace hgn {
class queue;
class device final {
  public:
    device(const device&) noexcept                    = delete;
    auto operator=(const device&) noexcept -> device& = delete;
    device(device&&) noexcept                         = default;
    auto operator=(device&&) noexcept -> device&      = default;
    ~device() noexcept                                = default;

    [[nodiscard]] auto get_queue(const std::uint32_t queue_family_index) noexcept -> queue;

    [[nodiscard]] auto make_semaphore() noexcept -> std::expected<semaphore, result> {
        auto info      = VkSemaphoreCreateInfo{};
        info.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        auto*      raw = static_cast<VkSemaphore_T*>(nullptr);
        const auto res = vkCreateSemaphore(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return semaphore{*raw, get()};
    }

    [[nodiscard]] auto make_fence() noexcept -> std::expected<fence, result> {
        auto info      = VkFenceCreateInfo{};
        info.sType     = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags     = VK_FENCE_CREATE_SIGNALED_BIT;
        auto*      raw = static_cast<VkFence_T*>(nullptr);
        const auto res = vkCreateFence(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return fence{*raw, get()};
    }

    [[nodiscard]] auto make_command_pool(const std::uint32_t queue_family_index) noexcept
        -> std::expected<command_pool, result> {
        auto info             = VkCommandPoolCreateInfo{};
        info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = queue_family_index;

        auto*      raw = static_cast<VkCommandPool_T*>(nullptr);
        const auto res = vkCreateCommandPool(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return command_pool{*raw, get()};
    }

    [[nodiscard]] auto allocate_command_buffer(command_pool& pool) noexcept
        -> std::expected<command_buffer, result> {
        auto info               = VkCommandBufferAllocateInfo{};
        info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 1;
        info.commandPool        = &pool.get();

        auto*      raw = static_cast<VkCommandBuffer_T*>(nullptr);
        const auto res = vkAllocateCommandBuffers(&get(), &info, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return command_buffer{*raw};
    }

    [[nodiscard]] auto get_image_view_builder() noexcept -> image_view::builder {
        return image_view::builder{get()};
    }

    [[nodiscard]] auto make_buffer(const std::uint32_t size) noexcept
        -> std::expected<buffer, result> {
        auto info        = VkBufferCreateInfo{};
        info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        info.size        = size;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto*      raw = static_cast<VkBuffer_T*>(nullptr);
        const auto res = vkCreateBuffer(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return buffer{*raw, get()};
    }

    [[nodiscard]] auto allocate_memory(
        const std::uint64_t allocation_size, const std::uint32_t memory_type_index
    ) noexcept -> std::expected<device_memory, result> {
        auto info            = VkMemoryAllocateInfo{};
        info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.allocationSize  = allocation_size;
        info.memoryTypeIndex = memory_type_index;

        auto*      raw = static_cast<VkDeviceMemory_T*>(nullptr);
        const auto res = vkAllocateMemory(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return device_memory{*raw, get()};
    }

    [[nodiscard]] auto make_shader(const std::span<const char> codes) noexcept
        -> std::expected<shader_module, result> {
        auto info     = VkShaderModuleCreateInfo{};
        info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = codes.size();

        // TODO Configure this!
        info.pCode = reinterpret_cast<const std::uint32_t*>(codes.data());

        auto*      raw = static_cast<VkShaderModule_T*>(nullptr);
        const auto res = vkCreateShaderModule(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return shader_module{*raw, get()};
    }

    [[nodiscard]] auto get_pipeline_layout() noexcept -> std::expected<pipeline_layout, result> {
        auto info                   = VkPipelineLayoutCreateInfo{};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount         = 0;
        info.pushConstantRangeCount = 0;

        auto*      raw = static_cast<VkPipelineLayout_T*>(nullptr);
        const auto res = vkCreatePipelineLayout(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return pipeline_layout{*raw, get()};
    }

    [[nodiscard]] auto get_graphics_pipeline_builder() noexcept -> graphics_pipeline_builder {
        return graphics_pipeline_builder{get()};
    }

    [[nodiscard]] auto wait_idle() noexcept -> std::expected<void, result> {
        const auto res = vkDeviceWaitIdle(&get());
        if (res != VK_SUCCESS) return make_unexpected(res);
        return {};
    }

    [[nodiscard]] auto get() noexcept -> VkDevice_T& { return raw_; }

    class builder final {
      public:
        explicit builder() noexcept = default;

        auto next(const physical_device_dynamic_rendering_features& next) noexcept -> builder& {
            dynamic_rendering_ = next;
            return *this;
        }

        auto queue(const device_queue& dev_queue) noexcept -> builder& {
            queue_ = dev_queue;
            return *this;
        }

        auto features(const physical_device_features& feature) noexcept -> builder& {
            feature_ = feature;
            return *this;
        }

        [[nodiscard]] auto build(physical_device& physical) const noexcept
            -> std::expected<device, result> {
            auto info  = VkDeviceCreateInfo{};
            info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            if (dynamic_rendering_) info.pNext = &dynamic_rendering_->get();
            if (queue_) {
                info.queueCreateInfoCount = 1;
                info.pQueueCreateInfos    = &queue_->info();
            }
            if (feature_) {
                info.pEnabledFeatures = &feature_->get();
            }
            info.enabledExtensionCount   = 1;
            constexpr auto* extension    = "VK_KHR_swapchain";
            info.ppEnabledExtensionNames = &extension;

            auto*      raw = static_cast<VkDevice_T*>(nullptr);
            const auto res = vkCreateDevice(&physical.get(), &info, nullptr, &raw);
            if (res != VK_SUCCESS) return make_unexpected(res);

            assert(raw != nullptr);
            return device{*raw};
        }

      private:
        std::optional<device_queue>                               queue_;
        std::optional<physical_device_features>                   feature_;
        std::optional<physical_device_dynamic_rendering_features> dynamic_rendering_;
    };

  private:
    friend class device::builder;

    explicit device(VkDevice_T& raw) noexcept : raw_{std::move(raw)} {}

    raii::device raw_;
};
}  // namespace hgn