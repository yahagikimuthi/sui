#pragma once

#include <cassert>
#include <cstddef>
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
    buffer(buffer&&) noexcept                         = default;
    auto operator=(buffer&&) noexcept -> buffer&;
    ~buffer() noexcept = default;

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return raw_; }

    [[nodiscard]] auto map() noexcept -> std::expected<std::span<std::byte>, result> {
        auto*      data = static_cast<void*>(nullptr);
        const auto res  = vmaMapMemory(&raw_.init_allocator(), &raw_.init_allocation(), &data);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(data != nullptr);
        return std::span{static_cast<std::byte*>(data), size_};
    }

  private:
    explicit buffer(
        raw::buffer&        raw,
        raw::allocator&     allocator,
        raw::allocation&    alloc,
        const std::uint64_t size
    ) noexcept
        : raw_{std::move(raw), allocator, alloc}, size_{size} {}

    raii::buffer  raw_;
    std::uint64_t size_;
};
}  // namespace hgn