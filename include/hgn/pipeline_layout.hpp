#pragma once

#include <utility>

#include "hgn/raii/rendering.hpp"
#include "hgn/raw.hpp"

namespace hgn {
class pipeline_layout final {
    friend class device;

  public:
    [[nodiscard]] auto get() noexcept -> raw::pipeline_layout& { return raw_; }

  private:
    explicit pipeline_layout(raw::pipeline_layout& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::pipeline_layout raw_;
};
}  // namespace hgn