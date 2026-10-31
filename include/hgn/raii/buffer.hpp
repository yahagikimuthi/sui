#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raw.hpp"

namespace hgn::raii {
class device_memory final {
  public:
    explicit device_memory(raw::device_memory&& native, raw::device& dev) noexcept  // NOLINT
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

    [[nodiscard]] auto get() noexcept -> raw::device_memory& { return *raw_; }

    [[nodiscard]] operator raw::device_memory&() noexcept { return get(); }

    [[nodiscard]] auto init_device() noexcept -> raw::device& { return dev_; }

  private:
    void destroy() noexcept {
        if (raw_) vkFreeMemory(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::device_memory&>  raw_;
    std::reference_wrapper<raw::device> dev_;
};

class buffer final {
  public:
    [[nodiscard]] explicit buffer(raw::buffer&& native, raw::device& dev) noexcept  // NOLINT
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

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return *raw_; }

    [[nodiscard]] operator raw::buffer&() noexcept { return get(); }

    [[nodiscard]] auto init_device() noexcept -> raw::device& { return dev_; }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyBuffer(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::buffer&>         raw_;
    std::reference_wrapper<raw::device> dev_;
};
}  // namespace hgn::raii