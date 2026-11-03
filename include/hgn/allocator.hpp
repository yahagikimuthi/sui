#pragma once

#include <cassert>
#include <expected>
#include <utility>

#include <hgn/detail/vk_mem_alloc.h>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/instance.hpp"
#include "hgn/physical.hpp"
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

class allocator_creator final {
    friend auto get_allocator_creator() noexcept -> allocator_creator;

  public:
    [[nodiscard]] auto instance(instance& ref) noexcept -> allocator_creator& {
        info_.instance = &ref.get();
        return *this;
    }

    [[nodiscard]] auto device(device& dev) noexcept -> allocator_creator& {
        info_.device = &dev.get();
        return *this;
    }

    [[nodiscard]] auto physical(physical_device& physical) noexcept -> allocator_creator& {
        info_.physicalDevice = &physical.get();
        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::expected<allocator, result> {
        auto*      raw = static_cast<raw::allocator*>(nullptr);
        const auto res = vmaCreateAllocator(&info_, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return allocator{*raw};
    }

  private:
    explicit allocator_creator() noexcept { info_.vulkanApiVersion = VK_API_VERSION_1_3; }

    VmaAllocatorCreateInfo info_{};
};

[[nodiscard]] inline auto get_allocator_creator() noexcept -> allocator_creator {
    return allocator_creator{};
}
}  // namespace hgn