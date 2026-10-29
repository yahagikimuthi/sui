#pragma once

#include "hgn2/raii/command.hpp"

namespace hgn {
class command_pool final {
    friend class device;

  public:
    [[nodiscard]] auto get() noexcept -> VkCommandPool_T& { return raw_; }

  private:
    explicit command_pool(VkCommandPool_T& raw, VkDevice_T& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::command_pool raw_;
};

class command_buffer final {
    friend class device;

  public:
  private:
    explicit command_buffer(VkCommandBuffer_T& raw) noexcept : raw_{std::move(raw)} {}

    raii::command_buffer raw_;
};
}  // namespace hgn