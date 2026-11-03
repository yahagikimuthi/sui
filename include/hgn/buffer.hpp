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

class device_memory final {
    friend class device;

  public:
    device_memory(const device_memory&) noexcept                    = delete;
    auto operator=(const device_memory&) noexcept -> device_memory& = delete;
    device_memory(device_memory&&) noexcept                         = default;
    auto operator=(device_memory&&) noexcept -> device_memory&      = default;
    ~device_memory() noexcept                                       = default;

    [[nodiscard]] auto bind_buffer(buffer& buffer) noexcept -> std::expected<void, result> {
        const auto res = vkBindBufferMemory(&raw_.init_device(), &buffer.get(), &get(), 0);
        if (res != VK_SUCCESS) return make_unexpected(res);

        return {};
    }

    template <std::uint64_t Size>
    [[nodiscard]] auto map() noexcept -> std::expected<std::span<std::byte, Size>, result> {
        auto* data = static_cast<void*>(nullptr);

        const auto res = vkMapMemory(&raw_.init_device(), &get(), 0, Size, 0, &data);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(data != nullptr);
        return std::span<std::byte, Size>(static_cast<std::byte*>(data), Size);
    }

    [[nodiscard]] auto get() noexcept -> raw::device_memory& { return raw_; }

  private:
    explicit device_memory(raw::device_memory& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::device_memory raw_;
};
}  // namespace hgn