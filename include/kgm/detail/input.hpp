#pragma once

#include <array>
#include <cstddef>

#include "kgm/enums.hpp"

namespace kgm::detail {
template <auto N>
    requires std::is_scoped_enum_v<decltype(N)>
class button_input final {
    using E                    = decltype(N);
    static constexpr auto size = static_cast<std::size_t>(N);

  public:
    button_input() noexcept = default;

    void update() noexcept {
        previous_ = current_;
        current_  = next_;
    }

    [[nodiscard]] auto get_action(const E e_idx) noexcept -> action {
        const auto idx = static_cast<std::size_t>(e_idx);
        if (current_[idx] and previous_[idx]) return action::repeat;
        if (current_[idx] and not previous_[idx]) return action::press;
        if (not current_[idx] and previous_[idx]) return action::release;
        return action::none;
    }

    void set(const E e_idx, int act) noexcept {
        const auto idx = static_cast<std::size_t>(e_idx);
        if (act == GLFW_PRESS)
            next_[idx] = true;
        else if (act == GLFW_RELEASE)
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
}  // namespace kgm::detail