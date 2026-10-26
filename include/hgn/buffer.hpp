#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <span>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"

namespace hgn {
class buffer_create_info final {
  public:
    explicit buffer_create_info() noexcept {
        native_.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        native_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        native_.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    auto size(std::size_t n) noexcept -> buffer_create_info& {
        native_.size = n;
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

class buffer final {
  public:
    [[nodiscard]] static auto try_make(device& dev, const buffer_create_info& info) noexcept
        -> std::expected<buffer, result> {
        auto*      native = static_cast<VkBuffer_T*>(nullptr);
        const auto res    = vkCreateBuffer(&dev.get(), &info.get(), nullptr, &native);
        if (res != VK_SUCCESS) return make_result(res);

        assert(native != nullptr);
        return buffer{*native, dev.get()};
    }

    buffer(const buffer&) noexcept                    = delete;
    auto operator=(const buffer&) noexcept -> buffer& = delete;

    buffer(buffer&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(buffer&& other) noexcept -> buffer& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;

        return *this;
    }

    ~buffer() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkBuffer_T& { return *native_; }

  private:
    explicit buffer(VkBuffer_T& native, VkDevice_T& dev) : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) vkDestroyBuffer(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkBuffer_T&>         native_;
    std::reference_wrapper<VkDevice_T> dev_;
};

[[nodiscard]] inline auto try_make_buffer(device& dev, const buffer_create_info& info) noexcept
    -> std::expected<buffer, result> {
    return buffer::try_make(dev, info);
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
    vkGetBufferMemoryRequirements(&dev.get(), &buffer_ref.get(), &native);
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

class device_memory final {
  public:
    [[nodiscard]] static auto try_allocate(device& dev, const memory_allocate_info& info) noexcept
        -> std::expected<device_memory, result> {
        auto*      native = static_cast<VkDeviceMemory_T*>(nullptr);
        const auto res    = vkAllocateMemory(&dev.get(), &info.get(), nullptr, &native);
        if (res != VK_SUCCESS) return make_result(res);

        assert(native != nullptr);
        return device_memory{*native, dev.get()};
    }

    device_memory(const device_memory&) noexcept                    = delete;
    auto operator=(const device_memory&) noexcept -> device_memory& = delete;

    device_memory(device_memory&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(device_memory&& other) noexcept -> device_memory& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;
        return *this;
    }

    ~device_memory() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkDeviceMemory_T& { return *native_; }

  private:
    explicit device_memory(VkDeviceMemory_T& native, VkDevice_T& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) vkFreeMemory(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkDeviceMemory_T&>   native_;
    std::reference_wrapper<VkDevice_T> dev_;
};

[[nodiscard]] inline auto try_allocate_memory(
    device& dev, const memory_allocate_info& info
) noexcept -> std::expected<device_memory, result> {
    return device_memory::try_allocate(dev, info);
}

[[nodiscard]] inline auto try_bind_buffer_memory(
    device& dev, buffer& buffer_ref, device_memory& dev_mem
) noexcept -> std::expected<void, result> {
    const auto res = vkBindBufferMemory(&dev.get(), &buffer_ref.get(), &dev_mem.get(), 0);
    if (res != VK_SUCCESS) return make_result(res);

    return {};
}

[[nodiscard]] inline auto try_map_memory(
    device& dev, device_memory& dev_mem, const std::uint64_t buffer_size
) noexcept -> std::expected<std::span<std::byte>, result> {
    auto*      data = static_cast<void*>(nullptr);
    const auto res  = vkMapMemory(&dev.get(), &dev_mem.get(), 0, buffer_size, 0, &data);
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
    vkUnmapMemory(&dev.get(), &dev_mem.get());
}
}  // namespace hgn