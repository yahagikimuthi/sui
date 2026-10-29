#pragma once

#include <optional>

#include <vulkan/vulkan.h>

namespace hgn {
class surface_khr final {
    friend class window;

  public:
  private:
    explicit surface_khr(VkSurfaceKHR_T& raw) noexcept : raw_{raw} {}

    std::optional<VkSurfaceKHR_T&> raw_;
};
}  // namespace hgn