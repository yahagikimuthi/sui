#pragma once

#include <cstdint>
#include <expected>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <GLFW/glfw3.h>

#include "akr/akr.hpp"
#include "kgm/window.hpp"

namespace kgm {
class window_creator final {
  public:
    window_creator() noexcept = default;

    auto extent(const std::uint32_t width, const std::uint32_t height) noexcept -> window_creator& {
        width_  = static_cast<int>(width);
        height_ = static_cast<int>(height);
        return *this;
    }

    auto title(const std::string_view title) noexcept -> window_creator& {
        title_ = title;
        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::expected<window, akr::error> {
        if (glfwInit() == 0) return akr::make_error(akr::runtime_error, "Failed to init GLFW.");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        auto* raw = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
        if (raw == nullptr) return akr::make_error(akr::runtime_error, "Failed to create Window.");

        return window{*raw};
    }

  private:
    int         width_{};
    int         height_{};
    std::string title_;
};

[[nodiscard]] inline auto get_required_instance_extensions() noexcept
    -> std::vector<std::string_view> {
    auto   count = std::uint32_t{};
    auto** ptrs  = glfwGetRequiredInstanceExtensions(&count);
    auto   span  = std::span{ptrs, count};
    return span | std::views::transform([](const char* ptr) noexcept -> std::string_view {
               return std::string_view{ptr};
           }) |
           std::ranges::to<std::vector>();
}
}  // namespace kgm