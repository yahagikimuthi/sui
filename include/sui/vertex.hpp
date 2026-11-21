#pragma once

#include "sui/color.hpp"
#include "sui/uv.hpp"
#include "sui/vec.hpp"

namespace sui {
struct vertex final {
    vec3 position{};
    rgba color{};
};

struct uv_vertex final {
    vec3 position;
    rgba color;
    uv   tex_coord;
};
}  // namespace sui