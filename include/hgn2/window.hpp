#pragma once

#include <expected>
#include <optional>

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
        -> std::expected<surface_khr, result>;

  private:
    std::optional<GLFWwindow&> raw_;
};
}  // namespace hgn