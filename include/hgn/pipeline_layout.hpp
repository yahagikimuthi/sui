#pragma once

#include <utility>

#include "hgn/raii/rendering.hpp"

namespace hgn {
class pipeline_layout final {
    friend class device;

  public:
    [[nodiscard]] auto get() noexcept -> VkPipelineLayout_T& { return raw_; }

  private:
    explicit pipeline_layout(VkPipelineLayout_T& raw, VkDevice_T& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::pipeline_layout raw_;
};
}  // namespace hgn