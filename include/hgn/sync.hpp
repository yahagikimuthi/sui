#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <limits>

#include <vulkan/vulkan.h>

#include "hgn/declaration.hpp"
#include "hgn/enums.hpp"

namespace hgn {
class semaphore_create_info final {
  public:
    explicit semaphore_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    }

  private:
    VkSemaphoreCreateInfo native_{};
};

class fence_create_info final {
  public:
    explicit fence_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        native_.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

  private:
    VkFenceCreateInfo native_{};
};

[[nodiscard]] inline auto try_make_semaphore(
    device& dev, const semaphore_create_info& info
) noexcept -> std::expected<std::reference_wrapper<semaphore>, result> {
    auto* sem         = static_cast<semaphore*>(nullptr);
    auto  native_info = std::bit_cast<VkSemaphoreCreateInfo>(info);

    auto res = vkCreateSemaphore(&dev, &native_info, nullptr, &sem);
    if (res != VK_SUCCESS) return make_result(res);

    assert(sem != nullptr);
    return std::ref(*sem);
}

inline void destroy_semaphore(device& dev, semaphore& target) noexcept {
    vkDestroySemaphore(&dev, &target, nullptr);
}

[[nodiscard]] inline auto try_make_fence(device& dev, const fence_create_info& info) noexcept
    -> std::expected<std::reference_wrapper<fence>, result> {
    auto* in_fence    = static_cast<fence*>(nullptr);
    auto  native_info = std::bit_cast<VkFenceCreateInfo>(info);

    auto res = vkCreateFence(&dev, &native_info, nullptr, &in_fence);
    if (res != VK_SUCCESS) return make_result(res);

    assert(in_fence != nullptr);
    return std::ref(*in_fence);
}

[[nodiscard]] inline auto try_wait_for_fence(
    device&             dev,
    fence&              fen,
    const bool          is_wait_all = true,
    const std::uint32_t timeout     = std::numeric_limits<std::uint32_t>::max()
) noexcept -> std::expected<void, result> {
    auto*      fen_ptr = &fen;
    const auto res =
        vkWaitForFences(&dev, 1, &fen_ptr, static_cast<VkBool32>(is_wait_all), timeout);

    if (res != VK_SUCCESS) return make_result(res);
    return {};
}

[[nodiscard]] inline auto try_reset_fence(device& dev, fence& fen) noexcept
    -> std::expected<void, result> {
    auto*      ptr = &fen;
    const auto res = vkResetFences(&dev, 1, &ptr);
    if (res != VK_SUCCESS) return make_result(res);
    return {};
}

inline void destroy_fence(device& dev, fence& target) noexcept {
    vkDestroyFence(&dev, &target, nullptr);
}
}  // namespace hgn