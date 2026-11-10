#pragma once

#include <cassert>
#include <expected>

#include <hgn/third_parties/vk_mem_alloc.h>

#include "hgn/allocator.hpp"
#include "hgn/detail/hgn.hpp"
#include "hgn/enums.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
[[nodiscard]] inline auto get_instance_creator() noexcept -> instance_creator {
    return instance_creator{};
}

class allocator_creator final {
  public:
    explicit allocator_creator() noexcept { info_.vulkanApiVersion = VK_API_VERSION_1_3; }

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
        auto* raw = static_cast<raw::allocator*>(nullptr);
        return make_expected(vmaCreateAllocator(&info_, &raw))
            .transform([&]() noexcept -> allocator { return allocator{*raw}; });
    }

  private:
    VmaAllocatorCreateInfo info_{};
};

[[nodiscard]] inline auto get_allocator_creator() noexcept -> allocator_creator {
    return allocator_creator{};
}

static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);
}  // namespace hgn