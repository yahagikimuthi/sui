#pragma once

#include "sui/color.hpp"
#include "sui/vec2.hpp"

namespace sui {
struct alignas(4) vertex final {
    vec2 position{};
    rgba color{};
};

static_assert(sizeof(vertex) == 12);
}  // namespace sui