#pragma once

#include <bit>
#include <cstdint>

#include <vulkan/vulkan.h>

#include "hgn2/enums.hpp"

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

class physical_device_features final {
  public:
    explicit physical_device_features() noexcept = default;

    [[nodiscard]] auto get() const noexcept -> const VkPhysicalDeviceFeatures& { return raw_; }

  private:
    VkPhysicalDeviceFeatures raw_{};
};

class physical_device_dynamic_rendering_features final {
  public:
    explicit physical_device_dynamic_rendering_features() noexcept {
        raw_.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        raw_.dynamicRendering = VK_TRUE;
    }

    [[nodiscard]] auto get() const noexcept -> const VkPhysicalDeviceDynamicRenderingFeatures& {
        return raw_;
    }

  private:
    VkPhysicalDeviceDynamicRenderingFeatures raw_{};
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
}  // namespace hgn