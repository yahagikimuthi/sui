#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#undef GLFW_INCLUDE_VULKAN

#include "hgn/enums.hpp"
#include "hgn/instance.hpp"
#include "hgn/raii/window.hpp"
#include "hgn/raw.hpp"
#include "hgn/surface.hpp"

namespace hgn {

enum class window_hint : std::int32_t { client_api = GLFW_CLIENT_API };
enum class window_value : std::uint8_t { no_api = GLFW_NO_API };

class window final {
  public:
    [[nodiscard]] static auto init() noexcept -> bool { return glfwInit() != GL_FALSE; }

    static void set_hint(const window_hint hint, const window_value value) noexcept {
        glfwWindowHint(std::to_underlying(hint), std::to_underlying(value));
    }

    [[nodiscard]] static auto make(
        const std::uint32_t width, const std::uint32_t height, const std::string_view title
    ) noexcept -> std::optional<window> {
        auto* raw = glfwCreateWindow(
            static_cast<std::int32_t>(width),
            static_cast<std::int32_t>(height),
            std::string{title}.c_str(),
            nullptr,
            nullptr
        );
        if (raw == nullptr) return std::nullopt;
        return window{*raw};
    }

    static void poll_events() noexcept { glfwPollEvents(); }

    [[nodiscard]] auto is_open() noexcept -> bool { return glfwWindowShouldClose(&get()) == 0; }

    [[nodiscard]] auto make_surface(instance& instance_ref) noexcept
        -> std::expected<surface_khr, result> {
        auto*      raw = static_cast<raw::surface_khr*>(nullptr);
        const auto res = glfwCreateWindowSurface(&instance_ref.get(), &get(), nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return surface_khr{*raw, instance_ref.get()};
    }

    [[nodiscard]] auto get() noexcept -> GLFWwindow& { return raw_; }

  private:
    explicit window(GLFWwindow& raw) noexcept : raw_{std::move(raw)} {}

    raii::window raw_;
};
}  // namespace hgn