#pragma once

#include <cassert>
#include <cstdint>
#include <expected>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw.hpp"
#include "hgn/surface.hpp"

namespace hgn {
class queue_family final {
    friend class physical_device;

  public:
    [[nodiscard]] auto flags() const noexcept -> hgn::queue_flag_bits {
        return static_cast<hgn::queue_flag_bits>(property_.queueFlags);
    }

    [[nodiscard]] auto is_supporting_surface_khr(surface_khr& surface) noexcept
        -> std::expected<bool, result> {
        auto       supported = VkBool32{};
        const auto res       = vkGetPhysicalDeviceSurfaceSupportKHR(
            &raw_.init_physical(), get(), &surface.get(), &supported
        );
        if (res != VK_SUCCESS) return make_unexpected(res);

        return supported != VK_FALSE;
    }

    [[nodiscard]] auto get() noexcept -> std::uint32_t { return raw_; }

  private:
    explicit queue_family(
        std::uint32_t                  index,
        raw::physical_device&          init_physical,
        const VkQueueFamilyProperties& property
    ) noexcept
        : raw_{index, init_physical}, property_{property} {}

    raii::queue_family      raw_;
    VkQueueFamilyProperties property_;
};
}  // namespace hgn