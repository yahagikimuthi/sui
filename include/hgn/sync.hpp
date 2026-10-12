#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <bit>
#include <cassert>
#include <expected>
#include <functional>
#include <limits>
#include <span>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"

namespace hgn {
using semaphore = VkSemaphore_T;
using fence     = VkFence_T;

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

[[nodiscard]] auto try_wait_for_fence(
    device&                 dev,
    std::span<fence* const> fences,
    const bool              is_wait_all = true,
    std::uint64_t           timeout     = std::numeric_limits<std::uint64_t>::max()
) noexcept -> std::expected<void, result> {
    const auto res = vkWaitForFences(
        &dev,
        static_cast<std::uint32_t>(fences.size()),
        fences.data(),
        static_cast<VkBool32>(is_wait_all),
        timeout
    );

    if (res != VK_SUCCESS) return make_result(res);
    return {};
}

inline void destroy_fence(device& dev, fence& target) noexcept {
    vkDestroyFence(&dev, &target, nullptr);
}
}  // namespace hgn