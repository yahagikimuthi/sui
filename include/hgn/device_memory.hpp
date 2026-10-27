#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <span>

#include <vulkan/vulkan.h>

#include "hgn/declaration.hpp"
#include "hgn/enums.hpp"

namespace hgn {
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