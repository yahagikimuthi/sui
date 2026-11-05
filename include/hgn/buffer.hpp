#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <span>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raii/buffer.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class buffer final {
    friend class buffer_creator;

  public:
    buffer(const buffer&) noexcept                     = delete;
    auto operator=(const buffer&) noexcept -> buffer&  = delete;
    buffer(buffer&& other) noexcept                    = default;
    auto operator=(buffer&& other) noexcept -> buffer& = default;

    ~buffer() noexcept = default;

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return raw_; }

    template <typename T>
    void cpy(const std::span<T> data) noexcept {
        assert(data_ != nullptr);
        std::memcpy(data_, data.data(), sizeof(T) * data.size());
    }

  private:
    explicit buffer(
        raw::buffer& raw, raw::allocator& allocator, raw::allocation& alloc, std::byte* data
    ) noexcept
        : raw_{std::move(raw), allocator, alloc}, data_{data} {}

    raii::buffer raw_;
    std::byte*   data_;
};
}  // namespace hgn