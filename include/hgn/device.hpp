#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <span>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/command.hpp"
#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/pipeline_layout.hpp"
#include "hgn/pipeline_structs.hpp"
#include "hgn/queue.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw.hpp"
#include "hgn/shader.hpp"
#include "hgn/structs.hpp"
#include "hgn/surface.hpp"
#include "hgn/swapchain.hpp"
#include "hgn/sync.hpp"

namespace hgn {
class swapchain_khr_builder final {
    friend class device;

  public:
    auto surface(surface_khr& surface) noexcept -> swapchain_khr_builder& {
        info_.surface = &surface.get();
        return *this;
    }

    auto min_image_count(const std::uint32_t count) noexcept -> swapchain_khr_builder& {
        info_.minImageCount = count;
        return *this;
    }

    auto image_format(const format fmt) noexcept -> swapchain_khr_builder& {
        info_.imageFormat = static_cast<VkFormat>(fmt);
        return *this;
    }

    auto image_color_space(const color_space_khr space) noexcept -> swapchain_khr_builder& {
        info_.imageColorSpace = static_cast<VkColorSpaceKHR>(space);
        return *this;
    }

    auto image_extent(const extent2d extent) noexcept -> swapchain_khr_builder& {
        info_.imageExtent = std::bit_cast<VkExtent2D>(extent);
        return *this;
    }

    auto pre_transform(const surface_transform_flag_bits_khr transform) noexcept
        -> swapchain_khr_builder& {
        info_.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(transform);
        return *this;
    }

    auto present_mode(const present_mode_khr mode) noexcept -> swapchain_khr_builder& {
        info_.presentMode = static_cast<VkPresentModeKHR>(mode);
        return *this;
    }

    auto queue_family_indices(const std::span<const std::uint32_t> indices) noexcept
        -> swapchain_khr_builder& {
        queue_family_indices_.assign_range(indices);
        return *this;
    }

    auto image_sharing_mode(const sharing_mode mode) noexcept -> swapchain_khr_builder& {
        info_.imageSharingMode = static_cast<VkSharingMode>(mode);
        return *this;
    }

    [[nodiscard]] auto build() noexcept -> std::expected<swapchain_khr, result> {
        info_.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_family_indices_.size());
        info_.pQueueFamilyIndices   = queue_family_indices_.data();

        auto*      raw = static_cast<VkSwapchainKHR_T*>(nullptr);
        const auto res = vkCreateSwapchainKHR(&dev_.get(), &info_, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return swapchain_khr{*raw, dev_.get(), static_cast<format>(info_.imageFormat)};
    }

  private:
    explicit swapchain_khr_builder(raw::device& dev) noexcept : dev_{std::ref(dev)} {
        info_.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info_.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info_.imageArrayLayers = 1;
        info_.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info_.clipped          = VK_TRUE;
    }

    VkSwapchainCreateInfoKHR            info_{};  // NOLINT
    std::vector<std::uint32_t>          queue_family_indices_;
    std::reference_wrapper<raw::device> dev_;
};

class device final {
    friend class physical_device;

  public:
    device(const device&) noexcept                    = delete;
    auto operator=(const device&) noexcept -> device& = delete;
    device(device&&) noexcept                         = default;
    auto operator=(device&&) noexcept -> device&      = default;
    ~device() noexcept                                = default;

    [[nodiscard]] auto make_queue(const std::uint32_t queue_family_index) noexcept -> queue {
        auto* raw = static_cast<raw::queue*>(nullptr);
        vkGetDeviceQueue(&get(), queue_family_index, 0, &raw);

        assert(raw != nullptr);
        return queue{*raw, queue_family_index};
    }

    [[nodiscard]] auto get_swapchain_builder() noexcept -> swapchain_khr_builder {
        return swapchain_khr_builder{get()};
    }

    [[nodiscard]] auto make_semaphore() noexcept -> std::expected<semaphore, result> {
        auto info  = VkSemaphoreCreateInfo{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        auto*      raw = static_cast<raw::semaphore*>(nullptr);
        const auto res = vkCreateSemaphore(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return semaphore{*raw, get()};
    }

    [[nodiscard]] auto make_fence() noexcept -> std::expected<fence, result> {
        auto info  = VkFenceCreateInfo{};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        auto*      raw = static_cast<raw::fence*>(nullptr);
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

        auto*      raw = static_cast<raw::command_pool*>(nullptr);
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

        auto*      raw = static_cast<raw::command_buffer*>(nullptr);
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

        auto*      raw = static_cast<raw::buffer*>(nullptr);
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

        auto*      raw = static_cast<raw::device_memory*>(nullptr);
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

        auto*      raw = static_cast<raw::shader_module*>(nullptr);
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

        auto*      raw = static_cast<raw::pipeline_layout*>(nullptr);
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

    [[nodiscard]] auto get() noexcept -> raw::device& { return raw_; }

  private:
    explicit device(raw::device& raw) noexcept : raw_{std::move(raw)} {}

    raii::device raw_;
};
}  // namespace hgn