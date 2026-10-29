#pragma once

#include <functional>

#include <vulkan/vulkan.h>

namespace hgn {
class physical_device final {
  public:
  private:
    explicit physical_device() noexcept;

    std::reference_wrapper<VkPhysicalDevice_T> raw_;
};
}  // namespace hgn