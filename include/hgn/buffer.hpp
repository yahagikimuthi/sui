#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <utility>

#include <hgn/third_parties/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "hgn/raii/buffer.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class buffer final {
  public:
    explicit buffer(
        raw::buffer& raw, raw::allocator& allocator, raw::allocation& alloc, std::byte* data
    ) noexcept
        : raw_{std::move(raw), allocator, alloc}, data_{data} {}

    [[nodiscard]] auto size() noexcept -> std::uint64_t { return info().size; }

    template <typename T>
    void cpy(const std::span<T> data) noexcept {
        assert(data_ != nullptr);
        std::memcpy(data_, data.data(), sizeof(T) * data.size());
    }

    template <typename T>
    void cpy(const T& data) noexcept {
        std::memcpy(data_, &data, sizeof(T));
    }

    void cpy(std::byte* data, std::size_t size) noexcept { std::memcpy(data_, data, size); }

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return raw_; }

  private:
    [[nodiscard]] auto info() noexcept -> VmaAllocationInfo {
        auto info = VmaAllocationInfo{};
        vmaGetAllocationInfo(&raw_.init_allocator(), &raw_.init_allocation(), &info);
        return info;
    }

    raii::buffer raw_;
    std::byte*   data_;
};
}  // namespace hgn