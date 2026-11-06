#pragma once

#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raii/hardware.hpp"
#include "hgn/raw/raw.hpp"

namespace kgm {
class window;
}  // namespace kgm

namespace hgn {
class surface_khr final {
  public:
    explicit surface_khr(raw::surface_khr& raw, raw::instance& instance) noexcept
        : raw_{std::move(raw), instance} {}

    [[nodiscard]] auto get() noexcept -> raw::surface_khr& { return raw_; }

  private:
    raii::surface_khr raw_;
};
}  // namespace hgn