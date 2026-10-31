#pragma once

#include <utility>

#include <vulkan/vulkan.h>

#include "hgn2/raii/rendering.hpp"

namespace hgn {
class graphics_pipeline final {
    friend class graphics_pipeline_builder;

  public:
  private:
    explicit graphics_pipeline(VkPipeline_T& raw, VkDevice_T& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::pipeline raw_;
};
}  // namespace hgn