#pragma once

#include <bit>
#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"

namespace hgn {
class queue_family_properties final {
  public:
    [[nodiscard]] auto flags() const noexcept -> queue_flag_bits {
        return static_cast<queue_flag_bits>(raw_.queueFlags);
    }

  private:
    VkQueueFamilyProperties raw_;
};

struct extent2d final {
    std::uint32_t width;
    std::uint32_t height;
};

class surface_capabilities_khr final {
  public:
    [[nodiscard]] auto current_extent() const noexcept -> extent2d {
        return std::bit_cast<extent2d>(raw_.currentExtent);
    }

    [[nodiscard]] auto min_image_count() const noexcept -> std::uint32_t {
        return raw_.minImageCount;
    }

    [[nodiscard]] auto max_image_count() const noexcept -> std::uint32_t {
        return raw_.maxImageCount;
    }

    [[nodiscard]] auto current_transform() const noexcept -> surface_transform_flag_bits_khr {
        return static_cast<surface_transform_flag_bits_khr>(raw_.currentTransform);
    }

  private:
    VkSurfaceCapabilitiesKHR raw_;
};

class surface_format_khr final {
  public:
    [[nodiscard]] auto format() const noexcept -> format {
        return static_cast<enum format>(raw_.format);
    }

    [[nodiscard]] auto color_space() const noexcept -> color_space_khr {
        return static_cast<color_space_khr>(raw_.colorSpace);
    }

  private:
    VkSurfaceFormatKHR raw_;
};

class memory_requirements final {
  public:
    [[nodiscard]] auto memory_type_bits() const noexcept -> std::uint32_t {
        return raw_.memoryTypeBits;
    }

    [[nodiscard]] auto size() const noexcept -> std::uint64_t { return raw_.size; }

  private:
    VkMemoryRequirements raw_;
};

class memory_type final {
  public:
    [[nodiscard]] auto property_flags() const noexcept -> memory_property_flags {
        return static_cast<memory_property_flags>(raw_.propertyFlags);
    }

  private:
    VkMemoryType raw_;
};

class physical_device_memory_properties final {
  public:
    [[nodiscard]] auto memory_types() const noexcept -> std::vector<memory_type> {
        const auto* ptr  = static_cast<const VkMemoryType*>(raw_.memoryTypes);
        auto        span = std::span{ptr, raw_.memoryTypeCount};
        return span | std::views::transform([](const VkMemoryType type) noexcept -> memory_type {
                   return std::bit_cast<memory_type>(type);
               }) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto memory_type_count() const noexcept -> std::uint32_t {
        return raw_.memoryTypeCount;
    }

  private:
    VkPhysicalDeviceMemoryProperties raw_;
};
}  // namespace hgn