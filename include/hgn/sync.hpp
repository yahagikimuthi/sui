#pragma once

#include <cstdint>
#include <expected>
#include <limits>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/raii/sync.hpp"
#include "hgn/raw.hpp"

namespace hgn {
class semaphore final {
    friend class device;

  public:
    semaphore(const semaphore&) noexcept                    = delete;
    auto operator=(const semaphore&) noexcept -> semaphore& = delete;
    semaphore(semaphore&&) noexcept                         = default;
    auto operator=(semaphore&&) noexcept -> semaphore&      = default;
    ~semaphore() noexcept                                   = default;

    [[nodiscard]] auto get() noexcept -> raw::semaphore& { return raw_; }

  private:
    explicit semaphore(raw::semaphore& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::semaphore raw_;
};

class fence final {
    friend class device;

  public:
    fence(const fence&) noexcept                    = delete;
    auto operator=(const fence&) noexcept -> fence& = delete;
    fence(fence&&) noexcept                         = default;
    auto operator=(fence&&) noexcept -> fence&      = default;

    ~fence() noexcept = default;

    [[nodiscard]] auto wait() noexcept -> std::expected<void, result> {
        auto*      ptr = &get();
        const auto res = vkWaitForFences(
            &raw_.init_device(), 1, &ptr, VK_TRUE, std::numeric_limits<std::uint32_t>::max()
        );

        if (res != VK_SUCCESS) return make_unexpected(res);
        return {};
    }

    [[nodiscard]] auto reset() noexcept -> std::expected<void, result> {
        auto*      ptr = &get();
        const auto res = vkResetFences(&raw_.init_device(), 1, &ptr);
        if (res != VK_SUCCESS) return make_unexpected(res);
        return {};
    }

    [[nodiscard]] auto get() noexcept -> raw::fence& { return raw_; }

  private:
    explicit fence(raw::fence& raw, raw::device& dev) noexcept : raw_{std::move(raw), dev} {}

    raii::fence raw_;
};
}  // namespace hgn