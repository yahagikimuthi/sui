#pragma once

#include <glm/ext/matrix_float4x4.hpp>

namespace sui {
struct uniform_buffer_object final {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
}  // namespace sui