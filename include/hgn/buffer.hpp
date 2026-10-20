#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <span>

#include <vulkan/vulkan.h>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"

namespace hgn {
using buffer        = VkBuffer_T;
using device_memory = VkDeviceMemory_T;

class buffer_create_info final {
  public:
    explicit buffer_create_info() noexcept {
        native_.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        native_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    auto size(std::size_t n) noexcept -> buffer_create_info& {
        native_.size = n;
        return *this;
    }

    auto usage(const buffer_usage_flag_bits flag) noexcept -> buffer_create_info& {
        native_.usage = static_cast<VkBufferUsageFlagBits>(flag);
        return *this;
    }

    auto setting_sharing_mode(const sharing_mode mode) noexcept -> buffer_create_info& {
        native_.sharingMode = static_cast<VkSharingMode>(mode);
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkBufferCreateInfo& { return native_; }

  private:
    VkBufferCreateInfo native_{};
};

[[nodiscard]] inline auto try_make_buffer(device& dev, const buffer_create_info& info) noexcept
    -> std::expected<std::reference_wrapper<buffer>, result> {
    auto*      ptr = static_cast<buffer*>(nullptr);
    const auto res = vkCreateBuffer(&dev, &info.get(), nullptr, &ptr);
    if (res != VK_SUCCESS) return make_result(res);

    assert(ptr != nullptr);
    return std::ref(*ptr);
}

inline void destroy_buffer(device& dev, buffer& target) noexcept {
    vkDestroyBuffer(&dev, &target, nullptr);
}

class memory_requirements final {
  public:
    [[nodiscard]] auto size() const noexcept -> std::uint64_t { return native_.size; }

    [[nodiscard]] auto memory_type_bits() const noexcept -> std::uint32_t {
        return native_.memoryTypeBits;
    }

  private:
    VkMemoryRequirements native_;
};

[[nodiscard]] inline auto get_buffer_memory_requirements(device& dev, buffer& buffer_ref) noexcept
    -> memory_requirements {
    auto native = VkMemoryRequirements{};
    vkGetBufferMemoryRequirements(&dev, &buffer_ref, &native);
    return std::bit_cast<memory_requirements>(native);
}

class memory_allocate_info final {
  public:
    explicit memory_allocate_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    }

    auto allocation_size(const std::uint64_t n) noexcept -> memory_allocate_info& {
        native_.allocationSize = n;
        return *this;
    }

    auto memory_type_index(std::uint32_t n) noexcept -> memory_allocate_info& {
        native_.memoryTypeIndex = n;
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkMemoryAllocateInfo& { return native_; }

  private:
    VkMemoryAllocateInfo native_{};
};

[[nodiscard]] inline auto try_allocate_memory(
    device& dev, const memory_allocate_info& info
) noexcept -> std::expected<std::reference_wrapper<device_memory>, result> {
    auto*      dev_mem = static_cast<device_memory*>(nullptr);
    const auto res     = vkAllocateMemory(&dev, &info.get(), nullptr, &dev_mem);
    if (res != VK_SUCCESS) return make_result(res);

    assert(dev_mem != nullptr);
    return std::ref(*dev_mem);
}

[[nodiscard]] inline auto try_bind_buffer_memory(
    device& dev, buffer& buffer_ref, device_memory& dev_mem
) noexcept -> std::expected<void, result> {
    const auto res = vkBindBufferMemory(&dev, &buffer_ref, &dev_mem, 0);
    if (res != VK_SUCCESS) return make_result(res);

    return {};
}

[[nodiscard]] inline auto try_map_memory(
    device& dev, device_memory& dev_mem, const std::uint64_t buffer_size
) noexcept -> std::expected<std::span<std::byte>, result> {
    auto*      data = static_cast<void*>(nullptr);
    const auto res  = vkMapMemory(&dev, &dev_mem, 0, buffer_size, 0, &data);
    if (res != VK_SUCCESS) return make_result(res);

    assert(data != nullptr);
    return std::span{static_cast<std::byte*>(data), buffer_size};
}

template <std::uint64_t BufferSize>
[[nodiscard]] inline auto try_map_memory(device& dev, device_memory& dev_mem) noexcept
    -> std::expected<std::span<std::byte, BufferSize>, result> {
    auto res = try_map_memory(dev, dev_mem, BufferSize);
    if (not res) return std::unexpected{res.error()};
    return std::span<std::byte, BufferSize>{*res};
}

inline void unmap_memory(device& dev, device_memory& dev_mem) noexcept {
    vkUnmapMemory(&dev, &dev_mem);
}

inline void free_memory(device& dev, device_memory& target) noexcept {
    vkFreeMemory(&dev, &target, nullptr);
}
}  // namespace hgn