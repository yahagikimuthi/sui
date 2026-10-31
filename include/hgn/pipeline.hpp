#pragma once

#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raii/rendering.hpp"
#include "hgn/raw.hpp"

namespace hgn {
class graphics_pipeline final {
    friend class graphics_pipeline_builder;

  public:
    [[nodiscard]] auto get() noexcept -> raw::pipeline& { return raw_; }

  private:
    explicit graphics_pipeline(raw::pipeline& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::pipeline raw_;
};
}  // namespace hgn