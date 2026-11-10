#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <ranges>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/physical.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/surface.hpp"

namespace hgn {
class instance final {
  public:
    explicit instance(raw::instance& raw) noexcept : raw_{std::move(raw)} {}

    // 物理デバイスの列挙
    [[nodiscard]] auto get_physicals() noexcept
        -> std::expected<std::vector<physical_device>, result> {
        auto count = std::uint32_t{};
        if (const auto res = vkEnumeratePhysicalDevices(&get(), &count, nullptr); res != VK_SUCCESS)
            return make_unexpected(res);

        auto devices = std::vector<raw::physical_device*>(count);
        if (const auto res = vkEnumeratePhysicalDevices(&get(), &count, devices.data());
            res != VK_SUCCESS)
            return make_unexpected(res);

        return devices |
               std::views::transform([](raw::physical_device* ptr) noexcept -> physical_device {
                   assert(ptr != nullptr);
                   return physical_device{*ptr};
               }) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto create_window_surface(GLFWwindow& win
    ) noexcept -> std::expected<surface_khr, result> {
        auto*      raw = static_cast<raw::surface_khr*>(nullptr);
        const auto res = glfwCreateWindowSurface(&get(), &win, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);
        return surface_khr{*raw, get()};
    }

    [[nodiscard]] auto get() noexcept -> raw::instance& { return raw_; }

  private:
    raii::instance raw_;
};
}  // namespace hgn