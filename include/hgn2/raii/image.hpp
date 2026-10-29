#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn::raii {
class swapchain_khr final {
  public:
    [[nodiscard]] explicit swapchain_khr(
        VkSwapchainKHR_T&& raw, VkDevice_T& dev  // NOLINT
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

  private:
    void destroy() noexcept {
        if (raw_) vkDestroySwapchainKHR(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkSwapchainKHR_T&>   raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};

class image final {
  public:
    [[nodiscard]] explicit image(VkImage_T&& raw) noexcept : raw_{raw} {}  // NOLINT

    image(const image&) noexcept                    = delete;
    auto operator=(const image&) noexcept -> image& = delete;
    image(image&&) noexcept                         = default;
    auto operator=(image&&) noexcept -> image&      = default;
    ~image() noexcept                               = default;

    [[nodiscard]] auto get() noexcept -> VkImage_T& { return raw_; }

    [[nodiscard]] operator VkImage_T&() noexcept { return get(); }

  private:
    std::reference_wrapper<VkImage_T> raw_;
};

class image_view final {
  public:
    [[nodiscard]] explicit image_view(VkImageView_T&& raw, VkDevice_T& dev) noexcept  // NOLINT
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

    [[nodiscard]] auto get() noexcept -> VkImageView_T& { return *raw_; }

    [[nodiscard]] operator VkImageView_T&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyImageView(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkImageView_T&>      raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};
}  // namespace hgn::raii