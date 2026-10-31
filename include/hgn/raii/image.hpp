#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raw.hpp"

namespace hgn::raii {
class swapchain_khr final {
  public:
    [[nodiscard]] explicit swapchain_khr(
        VkSwapchainKHR_T&& raw, raw::device& dev  // NOLINT
    ) noexcept
        : raw_{raw}, dev_{std::ref(dev)} {}

    swapchain_khr(const swapchain_khr&) noexcept                    = delete;
    auto operator=(const swapchain_khr&) noexcept -> swapchain_khr& = delete;

    swapchain_khr(swapchain_khr&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(swapchain_khr&& other) noexcept -> swapchain_khr& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~swapchain_khr() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkSwapchainKHR_T& { return *raw_; }

    [[nodiscard]] operator VkSwapchainKHR_T&() noexcept { return get(); }

    [[nodiscard]] auto init_device() noexcept -> raw::device& { return dev_; }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroySwapchainKHR(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkSwapchainKHR_T&>    raw_;
    std::reference_wrapper<raw::device> dev_;
};

class image final {
  public:
    [[nodiscard]] explicit image(raw::image&& raw) noexcept : raw_{raw} {}  // NOLINT

    image(const image&) noexcept                    = delete;
    auto operator=(const image&) noexcept -> image& = delete;
    image(image&&) noexcept                         = default;
    auto operator=(image&&) noexcept -> image&      = default;
    ~image() noexcept                               = default;

    [[nodiscard]] auto get() noexcept -> raw::image& { return raw_; }

    [[nodiscard]] operator raw::image&() noexcept { return get(); }

  private:
    std::reference_wrapper<raw::image> raw_;
};

class image_view final {
  public:
    [[nodiscard]] explicit image_view(raw::image_view&& raw, raw::device& dev) noexcept  // NOLINT
        : raw_{raw}, dev_{std::ref(dev)} {}

    image_view(const image_view&) noexcept                    = delete;
    auto operator=(const image_view&) noexcept -> image_view& = delete;

    image_view(image_view&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}
    auto operator=(image_view&& other) noexcept -> image_view& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~image_view() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> raw::image_view& { return *raw_; }

    [[nodiscard]] operator raw::image_view&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyImageView(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::image_view&>     raw_;
    std::reference_wrapper<raw::device> dev_;
};
}  // namespace hgn::raii