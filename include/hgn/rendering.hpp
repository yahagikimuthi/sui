#pragma once

#include <utility>

#include "hgn/raii/rendering.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class pipeline_layout final {
  public:
    explicit pipeline_layout(raw::pipeline_layout& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    [[nodiscard]] auto get() noexcept -> raw::pipeline_layout& { return raw_; }

  private:
    raii::pipeline_layout raw_;
};

class shader_module final {
  public:
    explicit shader_module(raw::shader_module& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    [[nodiscard]] auto get() noexcept -> raw::shader_module& { return raw_; }

  private:
    raii::shader_module raw_;
};

class graphics_pipeline final {
  public:
    explicit graphics_pipeline(raw::pipeline& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    [[nodiscard]] auto get() noexcept -> raw::pipeline& { return raw_; }

  private:
    raii::pipeline raw_;
};
}  // namespace hgn