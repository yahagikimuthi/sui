#pragma once

#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raii/hardware.hpp"
#include "hgn/raw.hpp"

namespace hgn {
class surface_khr final {
    friend class window;

  public:
    surface_khr(const surface_khr&) noexcept                    = delete;
    auto operator=(const surface_khr&) noexcept -> surface_khr& = delete;
    surface_khr(surface_khr&&) noexcept                         = default;
    auto operator=(surface_khr&&) noexcept -> surface_khr&      = default;
    ~surface_khr() noexcept                                     = default;

    [[nodiscard]] auto get() noexcept -> raw::surface_khr& { return raw_; }

  private:
    explicit surface_khr(raw::surface_khr& raw, raw::instance& instance) noexcept
        : raw_{std::move(raw), instance} {}

    raii::surface_khr raw_;
};
}  // namespace hgn