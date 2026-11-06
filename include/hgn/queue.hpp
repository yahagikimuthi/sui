#pragma once

#include <cstdint>
#include <expected>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/detail/queue.hpp"
#include "hgn/enums.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/surface.hpp"

namespace hgn {
class queue_family final {
    friend class physical_device;

  public:
    [[nodiscard]] auto index() const noexcept -> std::uint32_t { return raw_.index(); }

    [[nodiscard]] auto flags() noexcept -> queue_flag_bits {
        const auto out = get_properties().queueFlags;
        return static_cast<queue_flag_bits>(out);
    }

    [[nodiscard]] auto is_supporting_surface(surface_khr& surface
    ) noexcept -> std::expected<bool, result> {
        auto       supporting = VkBool32{};
        const auto res        = vkGetPhysicalDeviceSurfaceSupportKHR(
            &raw_.init_physical(), index(), &surface.get(), &supporting
        );
        if (res != VK_SUCCESS) return make_unexpected(res);
        return supporting != VK_FALSE;
    }

    [[nodiscard]] auto operator==(const queue_family& other) const noexcept -> bool {
        return index() == other.index();
    }

  private:
    explicit queue_family(std::uint32_t index, raw::physical_device& physical) noexcept
        : raw_{index, physical} {}

    [[nodiscard]] auto get_properties() noexcept -> VkQueueFamilyProperties {
        auto count = std::uint32_t{};
        vkGetPhysicalDeviceQueueFamilyProperties(&raw_.init_physical(), &count, nullptr);

        auto properties = std::vector<VkQueueFamilyProperties>(count);
        vkGetPhysicalDeviceQueueFamilyProperties(&raw_.init_physical(), &count, properties.data());

        return properties[index()];
    }

    raii::queue_family raw_;
};

class queue final {
    friend class device;

  public:
    [[nodiscard]] auto get_present_executor() noexcept -> present_executor {
        return present_executor{get()};
    }

    [[nodiscard]] auto get_submitter() noexcept -> submitter { return submitter{get()}; }

    [[nodiscard]] auto get() noexcept -> raw::queue& { return raw_; }

  private:
    explicit queue(raw::queue& raw) noexcept : raw_{std::move(raw)} {}

    raii::queue raw_;
};
}  // namespace hgn