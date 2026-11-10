#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <GLFW/glfw3.h>

#include "sui/input.hpp"

namespace sui::detail {
enum class action : std::uint8_t { none, down, pressed, released };

template <auto N>
class button_input final {
    using T                    = decltype(N);
    static constexpr auto size = static_cast<std::size_t>(N);

  public:
    explicit button_input() noexcept = default;

    void update() noexcept {
        previous_ = current_;
        current_  = next_;
    }

    [[nodiscard]] auto get_action(const T e_idx) noexcept -> action {
        const auto idx = static_cast<std::size_t>(e_idx);
        if (current_[idx] and previous_[idx]) return action::down;
        if (current_[idx] and not previous_[idx]) return action::pressed;
        if (not current_[idx] and previous_[idx]) return action::released;
        return action::none;
    }

    void set(const T e_idx, const int action) noexcept {
        const auto idx = static_cast<std::size_t>(e_idx);
        if (action == GLFW_PRESS)
            next_[idx] = true;
        else if (action == GLFW_RELEASE)
            next_[idx] = false;
    }

  private:
    std::array<bool, size> next_{};
    std::array<bool, size> current_{};
    std::array<bool, size> previous_{};
};

struct input final {
    void update() noexcept {
        keys.update();
        mouse_buttons.update();
    }

    button_input<key::last>          keys;
    button_input<mouse_button::last> mouse_buttons;
};
}  // namespace sui::detail