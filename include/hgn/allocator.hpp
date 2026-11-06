#pragma once

#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/detail/allocator.hpp"
#include "hgn/raii/command.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class allocator final {
    friend class allocator_creator;

  public:
    [[nodiscard]] auto get_buffer_creator() noexcept -> buffer_creator {
        return buffer_creator{get()};
    }

    [[nodiscard]] auto get() noexcept -> raw::allocator& { return raw_; }

  private:
    explicit allocator(raw::allocator& raw) noexcept : raw_{std::move(raw)} {}

    raii::allocator raw_;
};
}  // namespace hgn