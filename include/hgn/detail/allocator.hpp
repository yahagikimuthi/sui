#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/enums.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class buffer_creator final {
  public:
    explicit buffer_creator(raw::allocator& allocator) noexcept : allocator_{std::ref(allocator)} {
        info_.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    auto size(const std::uint64_t size) noexcept -> buffer_creator& {
        info_.size = size;
        return *this;
    }

    auto usage(const buffer_usage_flag_bits usage_bit) noexcept -> buffer_creator& {
        info_.usage = static_cast<VkBufferUsageFlagBits>(usage_bit);
        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::expected<buffer, result> {
        const auto alloc_create_info = VmaAllocationCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                     VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage          = VMA_MEMORY_USAGE_AUTO,
            .requiredFlags  = 0,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool           = nullptr,
            .pUserData      = nullptr,
            .priority       = 0,
            .minAlignment   = 0
        };

        auto*      raw            = static_cast<raw::buffer*>(nullptr);
        auto*      raw_allocation = static_cast<raw::allocation*>(nullptr);
        auto       alloc_info     = VmaAllocationInfo{};
        const auto res1           = vmaCreateBuffer(
            &allocator_.get(), &info_, &alloc_create_info, &raw, &raw_allocation, &alloc_info
        );
        if (res1 != VK_SUCCESS) return make_unexpected(res1);

        assert(raw != nullptr);
        assert(raw_allocation != nullptr);

        return buffer{
            *raw, allocator_.get(), *raw_allocation, static_cast<std::byte*>(alloc_info.pMappedData)
        };
    }

  private:
    VkBufferCreateInfo                     info_{};
    std::reference_wrapper<raw::allocator> allocator_;
};
}  // namespace hgn