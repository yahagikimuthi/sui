#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn::raii {
class device_memory final {
  public:
    explicit device_memory(VkDeviceMemory_T&& native, VkDevice_T& dev) noexcept  // NOLINT
        : raw_{native}, dev_{std::ref(dev)} {}

    device_memory(const device_memory&) noexcept                    = delete;
    auto operator=(const device_memory&) noexcept -> device_memory& = delete;

    device_memory(device_memory&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(device_memory&& other) noexcept -> device_memory& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~device_memory() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkDeviceMemory_T& { return *raw_; }

    [[nodiscard]] operator VkDeviceMemory_T&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkFreeMemory(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkDeviceMemory_T&>   raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};

class buffer final {
  public:
    explicit buffer(VkBuffer_T& native, VkDevice_T& dev) noexcept
        : raw_{native}, dev_{std::ref(dev)} {}

    buffer(const buffer&) noexcept                    = delete;
    auto operator=(const buffer&) noexcept -> buffer& = delete;

    buffer(buffer&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(buffer&& other) noexcept -> buffer& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~buffer() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkBuffer_T& { return *raw_; }

    [[nodiscard]] operator VkBuffer_T&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyBuffer(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkBuffer_T&>         raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};
}  // namespace hgn::raii