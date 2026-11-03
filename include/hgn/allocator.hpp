#pragma once

#include <utility>

#include <hgn/detail/vk_mem_alloc.h>

#include "hgn/raii/command.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class allocator final {
    friend class allocator_creator;

  public:
    [[nodiscard]] auto get() noexcept -> raw::allocator& { return raw_; }

  private:
    explicit allocator(raw::allocator& raw) noexcept : raw_{std::move(raw)} {}

    raii::allocator raw_;
};
}  // namespace hgn