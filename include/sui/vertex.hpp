#pragma once

#include "sui/color.hpp"
#include "sui/uv.hpp"
#include "sui/vec2.hpp"

namespace sui {
struct vertex final {
    vec2 position{};
    rgba color{};
};

struct uv_vertex final {
    vec2 position;
    rgba color;
    uv   tex_coord;
};

static_assert(sizeof(vertex) == 12);
static_assert(sizeof(uv_vertex) == 20);
}  // namespace sui