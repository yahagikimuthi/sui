#pragma once

#include <cassert>
#include <expected>
#include <optional>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#undef GLFW_INCLUDE_VULKAN

#include "hgn2/enums.hpp"
#include "hgn2/instance.hpp"
#include "hgn2/surface.hpp"

namespace hgn {
class window final {
  public:
    [[nodiscard]] auto get() noexcept -> GLFWwindow& { return *raw_; }

    [[nodiscard]] auto make_surface(instance& instance_ref) noexcept
        -> std::expected<surface_khr, result> {
        auto*      raw = static_cast<VkSurfaceKHR_T*>(nullptr);
        const auto res = glfwCreateWindowSurface(&instance_ref.get(), &get(), nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return surface_khr{*raw, instance_ref.get()};
    }

  private:
    std::optional<GLFWwindow&> raw_;
};
}  // namespace hgn