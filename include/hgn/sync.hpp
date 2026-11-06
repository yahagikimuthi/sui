#pragma once

#include <cstdint>
#include <expected>
#include <limits>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/raii/sync.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class semaphore final {
  public:
    explicit semaphore(raw::semaphore& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    [[nodiscard]] auto get() noexcept -> raw::semaphore& { return raw_; }

  private:
    raii::semaphore raw_;
};

class fence final {
  public:
    explicit fence(raw::fence& raw, raw::device& dev) noexcept : raw_{std::move(raw), dev} {}

    [[nodiscard]] auto wait() noexcept -> std::expected<void, result> {
        auto* ptr = &get();
        return make_expected(vkWaitForFences(
            &raw_.init_device(), 1, &ptr, VK_TRUE, std::numeric_limits<std::uint32_t>::max()
        ));
    }

    [[nodiscard]] auto reset() noexcept -> std::expected<void, result> {
        auto* ptr = &get();
        return make_expected(vkResetFences(&raw_.init_device(), 1, &ptr));
    }

    [[nodiscard]] auto get() noexcept -> raw::fence& { return raw_; }

  private:
    raii::fence raw_;
};
}  // namespace hgn