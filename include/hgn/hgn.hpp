#pragma once

#include "hgn/allocator.hpp"
#include "hgn/detail/hgn.hpp"

namespace hgn {
[[nodiscard]] inline auto get_instance_creator() noexcept -> instance_creator {
    return instance_creator{};
}

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

static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);
}  // namespace hgn