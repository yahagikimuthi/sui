#pragma once

#include <utility>

#include <vulkan/vulkan.h>

#include "hgn2/raii/sync.hpp"

namespace hgn {
class semaphore final {
    friend class device;

  public:
    semaphore(const semaphore&) noexcept                    = delete;
    auto operator=(const semaphore&) noexcept -> semaphore& = delete;
    semaphore(semaphore&&) noexcept                         = default;
    auto operator=(semaphore&&) noexcept -> semaphore&      = default;
    ~semaphore() noexcept                                   = default;

    [[nodiscard]] auto get() noexcept -> VkSemaphore_T& { return raw_; }

  private:
    explicit semaphore(VkSemaphore_T& raw, VkDevice_T& dev) noexcept : raw_{std::move(raw), dev} {}

    raii::semaphore raw_;
};

class fence final {
    friend class device;

  public:
    fence(const fence&) noexcept                    = delete;
    auto operator=(const fence&) noexcept -> fence& = delete;
    fence(fence&&) noexcept                         = default;
    auto operator=(fence&&) noexcept -> fence&      = default;

    ~fence() noexcept = default;

    [[nodiscard]] auto get() noexcept -> VkFence_T& { return raw_; }

  private:
    explicit fence(VkFence_T& raw, VkDevice_T& dev) noexcept : raw_{std::move(raw), dev} {}

    raii::fence raw_;
};
}  // namespace hgn