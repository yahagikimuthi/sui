#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/declaration.hpp"
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
}  // namespace hgn