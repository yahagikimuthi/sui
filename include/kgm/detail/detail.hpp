#pragma once

#include <functional>

#include "kgm/enums.hpp"

namespace kgm::detail {
struct callbacks final {
    std::function<void(key, action)>          keys_;
    std::function<void(mouse_button, action)> mouse_buttons;
    std::function<void(float, float)>         cursor_poss;
};
}  // namespace kgm::detail