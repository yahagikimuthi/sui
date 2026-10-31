#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <expected>
#include <span>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/raii/buffer.hpp"
#include "hgn/raw.hpp"
#include "hgn/structs.hpp"

namespace hgn {
class buffer final {
    friend class device;

  public:
    buffer(const buffer&) noexcept                    = delete;
    auto operator=(const buffer&) noexcept -> buffer& = delete;
    buffer(buffer&&) noexcept                         = default;
    auto operator=(buffer&&) noexcept -> buffer&;
    ~buffer() noexcept = default;

    [[nodiscard]] auto get_memory_requirements() noexcept -> memory_requirements {
        auto requirements = VkMemoryRequirements{};
        vkGetBufferMemoryRequirements(&raw_.init_device(), &get(), &requirements);
        return std::bit_cast<memory_requirements>(requirements);
    }

    [[nodiscard]] auto get() noexcept -> raw::buffer& { return raw_; }

  private:
    explicit buffer(raw::buffer& raw, raw::device& dev) noexcept : raw_{std::move(raw), dev} {}

    raii::buffer raw_;
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