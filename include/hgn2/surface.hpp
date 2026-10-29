#pragma once

#include <utility>

#include "hgn2/raii/hardware.hpp"

#include <vulkan/vulkan.h>

namespace hgn {
class surface_khr final {
    friend class window;

  public:
    surface_khr(const surface_khr&) noexcept                    = delete;
    auto operator=(const surface_khr&) noexcept -> surface_khr& = delete;
    surface_khr(surface_khr&&) noexcept                         = default;
    auto operator=(surface_khr&&) noexcept -> surface_khr&      = default;
    ~surface_khr() noexcept                                     = default;

    [[nodiscard]] auto get() noexcept -> VkSurfaceKHR_T& { return raw_; }

  private:
    explicit surface_khr(VkSurfaceKHR_T& raw, VkInstance_T& instance) noexcept
        : raw_{std::move(raw), instance} {}

    raii::surface_khr raw_;
};
}  // namespace hgn