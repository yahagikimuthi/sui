#pragma once

#include <expected>
#include <utility>

#include <hgn/detail/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/enums.hpp"
#include "hgn/raii/command.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class allocator final {
    friend class allocator_creator;

  public:
    [[nodiscard]] auto create_buffer(const std::uint64_t size) -> std::expected<buffer, result> {
        auto info        = VkBufferCreateInfo{};
        info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        info.size        = size;

        auto alloc_create_info = VmaAllocationCreateInfo{};
        alloc_create_info.requiredFlags =
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        auto*      raw            = static_cast<raw::buffer*>(nullptr);
        auto*      raw_allocation = static_cast<raw::allocation*>(nullptr);
        auto       alloc_info     = VmaAllocationInfo{};
        const auto res =
            vmaCreateBuffer(&get(), &info, &alloc_create_info, &raw, &raw_allocation, &alloc_info);

        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        assert(raw_allocation != nullptr);
        return buffer{*raw, get(), *raw_allocation, alloc_info.size};
    }

    [[nodiscard]] auto get() noexcept -> raw::allocator& { return raw_; }

  private:
    explicit allocator(raw::allocator& raw) noexcept : raw_{std::move(raw)} {}

    raii::allocator raw_;
};
}  // namespace hgn