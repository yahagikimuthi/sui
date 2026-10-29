#pragma once

#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn {
class surface_khr final {
    friend class window;

  public:
    surface_khr(const surface_khr&) noexcept                    = delete;
    auto operator=(const surface_khr&) noexcept -> surface_khr& = delete;

    surface_khr(surface_khr&& other) noexcept : raw_{std::exchange(other.raw_, std::nullopt)} {}

    auto operator=(surface_khr&& other) noexcept -> surface_khr& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        return *this;
    }

    ~surface_khr() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkSurfaceKHR_T& { return *raw_; }

  private:
    explicit surface_khr(VkSurfaceKHR_T& raw) noexcept : raw_{raw} {}

    void destroy() noexcept;

    std::optional<VkSurfaceKHR_T&> raw_;
};
}  // namespace hgn