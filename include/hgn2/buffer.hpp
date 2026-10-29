#pragma once

#include <functional>
#include <optional>

#include <vulkan/vulkan.h>

namespace hgn {
class buffer final {
  public:
    class builder final {};

  private:
    friend class buffer::builder;

    explicit buffer(VkBuffer_T& raw, VkDevice_T& dev) noexcept : raw_{raw}, dev_{std::ref(dev)} {}

    std::optional<VkBuffer_T&>         raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};
}  // namespace hgn