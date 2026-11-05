#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <expected>
#include <span>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/raii/buffer.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class buffer final {
    friend class allocator;

  public:
    buffer(const buffer&) noexcept                    = delete;
    auto operator=(const buffer&) noexcept -> buffer& = delete;

    buffer(buffer&& other) noexcept
        : raw_{std::move(other.raw_)}, data_{std::exchange(other.data_, nullptr)} {}

    auto operator=(buffer&& other) noexcept -> buffer& {
        if (this == &other) return *this;

        destroy();

        raw_  = std::move(other.raw_);
        data_ = std::exchange(other.data_, nullptr);
        return *this;
    }

    ~buffer() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return raw_; }

    [[nodiscard]] auto map() noexcept -> std::expected<void, result> {
        assert(data_ != nullptr);

        auto*      ptr = static_cast<void*>(data_);
        const auto res = vmaMapMemory(&raw_.init_allocator(), &raw_.init_allocation(), &ptr);
        if (res != VK_SUCCESS) return make_unexpected(res);

        return {};
    }

    template <typename T>
    void cpy(const std::span<const T> data) noexcept {
        assert(data_ == nullptr);
        std::memcpy(data_, data.data(), sizeof(T) * data.size());
    }

  private:
    explicit buffer(raw::buffer& raw, raw::allocator& allocator, raw::allocation& alloc) noexcept
        : raw_{std::move(raw), allocator, alloc} {}

    void destroy() noexcept {
        if (data_ != nullptr) vmaUnmapMemory(&raw_.init_allocator(), &raw_.init_allocation());
    }

    raii::buffer raw_;
    std::byte*   data_{nullptr};
};
}  // namespace hgn