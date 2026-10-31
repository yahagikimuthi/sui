#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn::raii {
class semaphore final {
  public:
    [[nodiscard]] explicit semaphore(VkSemaphore_T&& raw, VkDevice_T& dev) noexcept  // NOLINT
        : raw_{raw}, dev_{std::ref(dev)} {}

    semaphore(const semaphore&)                             = delete;
    auto operator=(const semaphore&) noexcept -> semaphore& = delete;

    semaphore(semaphore&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(semaphore&& other) noexcept -> semaphore& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~semaphore() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkSemaphore_T& { return *raw_; }

    [[nodiscard]] operator VkSemaphore_T&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroySemaphore(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkSemaphore_T&>      raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};

class fence final {
  public:
    [[nodiscard]] explicit fence(VkFence_T&& raw, VkDevice_T& dev) noexcept  // NOLINT
        : raw_{raw}, dev_{std::ref(dev)} {}

    fence(const fence&) noexcept                    = delete;
    auto operator=(const fence&) noexcept -> fence& = delete;

    fence(fence&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(fence&& other) noexcept -> fence& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;

        return *this;
    }

    ~fence() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkFence_T& { return *raw_; }

    [[nodiscard]] operator VkFence_T&() noexcept { return get(); }

    [[nodiscard]] auto init_device() noexcept -> VkDevice_T& { return dev_; }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyFence(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkFence_T&>          raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};
}  // namespace hgn::raii