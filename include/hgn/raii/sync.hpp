#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raw.hpp"

namespace hgn::raii {
class semaphore final {
  public:
    [[nodiscard]] explicit semaphore(raw::semaphore&& raw, raw::device& dev) noexcept  // NOLINT
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

    [[nodiscard]] auto get() noexcept -> raw::semaphore& { return *raw_; }

    [[nodiscard]] operator raw::semaphore&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroySemaphore(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::semaphore&>      raw_;
    std::reference_wrapper<raw::device> dev_;
};

class fence final {
  public:
    [[nodiscard]] explicit fence(raw::fence&& raw, raw::device& dev) noexcept  // NOLINT
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

    [[nodiscard]] auto get() noexcept -> raw::fence& { return *raw_; }

    [[nodiscard]] operator raw::fence&() noexcept { return get(); }

    [[nodiscard]] auto init_device() noexcept -> raw::device& { return dev_; }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyFence(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::fence&>          raw_;
    std::reference_wrapper<raw::device> dev_;
};
}  // namespace hgn::raii