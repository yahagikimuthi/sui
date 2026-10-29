#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn::raii {
class command_pool final {
  public:
    command_pool(const command_pool&) noexcept                    = delete;
    auto operator=(const command_pool&) noexcept -> command_pool& = delete;

    command_pool(command_pool&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(command_pool&& other) noexcept -> command_pool& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~command_pool() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkCommandPool_T& { return *raw_; }

    [[nodiscard]] operator VkCommandPool_T&() noexcept { return get(); }

  private:
    explicit command_pool(VkCommandPool_T& raw, VkDevice_T& dev) noexcept
        : raw_{raw}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (raw_) vkDestroyCommandPool(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkCommandPool_T&>    raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};

class command_buffer final {
  public:
    command_buffer(const command_buffer&) noexcept                   = delete;
    auto operator=(const command_buffer&) noexcept -> command_buffer = delete;
    command_buffer(command_buffer&&) noexcept                        = default;
    auto operator=(command_buffer&&) noexcept -> command_buffer&     = default;
    ~command_buffer() noexcept                                       = default;

    [[nodiscard]] auto get() noexcept -> VkCommandBuffer_T& { return raw_; }

    [[nodiscard]] operator VkCommandBuffer_T&() noexcept { return get(); }

  private:
    explicit command_buffer(VkCommandBuffer_T& raw) noexcept : raw_{std::ref(raw)} {}

    std::reference_wrapper<VkCommandBuffer_T> raw_;
};
}  // namespace hgn::raii