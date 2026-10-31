#pragma once

#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raii/rendering.hpp"

namespace hgn {
class graphics_pipeline final {
    friend class graphics_pipeline_builder;

  public:
    [[nodiscard]] auto get() noexcept -> VkPipeline_T& { return raw_; }

  private:
    explicit graphics_pipeline(VkPipeline_T& raw, VkDevice_T& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::pipeline raw_;
};
}  // namespace hgn