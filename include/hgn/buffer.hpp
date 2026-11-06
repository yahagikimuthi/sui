#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <utility>

#include <hgn/detail/vk_mem_alloc.h>
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

    [[nodiscard]] auto size() noexcept -> std::uint64_t { return info().size; }

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return raw_; }

    template <typename T>
    void cpy(const std::span<T> data) noexcept {
        assert(data_ != nullptr);
        std::memcpy(data_, data.data(), sizeof(T) * data.size());
    }

    template <typename T>
    void cpy(T& data) noexcept {
        std::memcpy(data_, &data, sizeof(T));
    }

  private:
    explicit buffer(
        raw::buffer& raw, raw::allocator& allocator, raw::allocation& alloc, std::byte* data
    ) noexcept
        : raw_{std::move(raw), allocator, alloc}, data_{data} {}

    [[nodiscard]] auto info() noexcept -> VmaAllocationInfo {
        auto info = VmaAllocationInfo{};
        vmaGetAllocationInfo(&raw_.init_allocator(), &raw_.init_allocation(), &info);
        return info;
    }

    raii::buffer raw_;
    std::byte*   data_;
};
}  // namespace hgn