#pragma once

#include <bit>
#include <cstdint>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"

namespace hgn {
using extent2d = VkExtent2D;

class surface_capabilities_khr final {
  public:
    [[nodiscard]] auto current_extent() const noexcept -> extent2d {
        return std::bit_cast<extent2d>(raw_.currentExtent);
    }

    [[nodiscard]] auto min_image_extent() const noexcept -> extent2d { return raw_.minImageExtent; }

    [[nodiscard]] auto max_image_extent() const noexcept -> extent2d { return raw_.maxImageExtent; }

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
}  // namespace hgn