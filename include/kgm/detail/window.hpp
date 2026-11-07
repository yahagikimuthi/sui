#pragma once

#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "kgm/enums.hpp"
#include "kgm/window.hpp"

namespace kgm {
template <typename User>
class window_creator final {
  public:
    explicit window_creator() noexcept = default;

    auto hint(const window_hint hint, const window_value value) noexcept -> window_creator& {
        glfwWindowHint(static_cast<std::int32_t>(hint), static_cast<std::int32_t>(value));
        return *this;
    }

    auto extent(const std::uint32_t width, const std::uint32_t height) noexcept -> window_creator& {
        width_  = static_cast<std::int32_t>(width);
        height_ = static_cast<std::int32_t>(height);
        return *this;
    }

    auto title(const std::string_view title) noexcept -> window_creator& {
        title_ = title;
        return *this;
    }

    [[nodiscard]] auto create() noexcept -> std::optional<window<User>> {
        auto* raw = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
        if (raw == nullptr) return std::nullopt;

        assert(raw != nullptr);
        return window<User>{*raw};
    }

  private:
    std::int32_t width_{800};
    std::int32_t height_{600};
    std::string  title_{"no title"};
};
}  // namespace kgm