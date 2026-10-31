#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raw.hpp"

namespace hgn::raii {
class command_pool final {
  public:
    explicit command_pool(raw::command_pool&& raw, raw::device& dev) noexcept  // NOLINT
        : raw_{raw}, dev_{std::ref(dev)} {}

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

    [[nodiscard]] auto get() noexcept -> raw::command_pool& { return *raw_; }

    [[nodiscard]] operator raw::command_pool&() noexcept { return get(); }

    [[nodiscard]] auto init_device() noexcept -> raw::device& { return dev_; }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyCommandPool(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::command_pool&>   raw_;
    std::reference_wrapper<raw::device> dev_;
};

class command_buffer final {
  public:
    explicit command_buffer(raw::command_buffer&& raw) noexcept : raw_{std::ref(raw)} {}  // NOLINT

    command_buffer(const command_buffer&) noexcept                   = delete;
    auto operator=(const command_buffer&) noexcept -> command_buffer = delete;
    command_buffer(command_buffer&&) noexcept                        = default;
    auto operator=(command_buffer&&) noexcept -> command_buffer&     = default;
    ~command_buffer() noexcept                                       = default;

    [[nodiscard]] auto get() noexcept -> raw::command_buffer& { return raw_; }

    [[nodiscard]] operator raw::command_buffer&() noexcept { return get(); }

  private:
    std::reference_wrapper<raw::command_buffer> raw_;
};
}  // namespace hgn::raii