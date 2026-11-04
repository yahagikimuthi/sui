#pragma once

#include <cstddef>
#include <functional>

#include <hgn/detail/vk_mem_alloc.h>

#include "hgn/raw/raw.hpp"

namespace hgn::raii {
class mapped_data final {
  public:
    explicit mapped_data(
        std::byte* data, raw::allocator& allocator, raw::allocation& alloc
    ) noexcept
        : data_{data}, allocator_{std::ref(allocator)}, allocation_{std::ref(alloc)} {}

    [[nodiscard]] auto get() const noexcept -> std::byte* { return data_; }

    [[nodiscard]] operator std::byte*() const noexcept { return get(); }

  private:
    void destroy() noexcept {}

    std::byte*                              data_;
    std::reference_wrapper<raw::allocator>  allocator_;
    std::reference_wrapper<raw::allocation> allocation_;
};
}  // namespace hgn::raii