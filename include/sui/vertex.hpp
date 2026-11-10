#pragma once

#include <array>

namespace sui {
struct vertex final {
    std::array<float, 2> position;
    std::array<float, 3> color;
};
}  // namespace sui