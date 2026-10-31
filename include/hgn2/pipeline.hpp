#pragma once

#include <vulkan/vulkan.h>

#include "hgn2/raii/rendering.hpp"

namespace hgn {
class graphics_pipeline final {
  public:
  private:
    explicit graphics_pipeline() noexcept;

    raii::pipeline raw_;
};
}  // namespace hgn