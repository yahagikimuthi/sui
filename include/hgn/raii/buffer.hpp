#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raw/raw.hpp"

namespace hgn::raii {
class buffer final {
  public:
    [[nodiscard]] explicit buffer(
        raw::buffer&& native, raw::allocator& allocator, raw::allocation& alloc  // NOLINT
    ) noexcept
        : raw_{native}, allocator_{std::ref(allocator)}, alloc_{std::ref(alloc)} {}

    buffer(const buffer&) noexcept                    = delete;
    auto operator=(const buffer&) noexcept -> buffer& = delete;

    buffer(buffer&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)},
          allocator_{other.allocator_},
          alloc_{other.alloc_} {}

    auto operator=(buffer&& other) noexcept -> buffer& {
        if (this == &other) return *this;

        destroy();

        raw_       = std::exchange(other.raw_, std::nullopt);
        allocator_ = other.allocator_;
        alloc_     = other.alloc_;
        return *this;
    }

    ~buffer() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return *raw_; }

    [[nodiscard]] operator raw::buffer&() noexcept { return get(); }

    [[nodiscard]] auto init_allocator() noexcept -> raw::allocator& { return allocator_; }
    [[nodiscard]] auto init_allocation() noexcept -> raw::allocation& { return alloc_; }

  private:
    void destroy() noexcept {
        if (raw_) vmaDestroyBuffer(&allocator_.get(), &get(), &alloc_.get());
    }

    std::optional<raw::buffer&>             raw_;
    std::reference_wrapper<raw::allocator>  allocator_;
    std::reference_wrapper<raw::allocation> alloc_;
};
}  // namespace hgn::raii