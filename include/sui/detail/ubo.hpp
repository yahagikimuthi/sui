#pragma once

#include <glm/glm.hpp>

namespace sui {
struct uniform_buffer_object final {
    glm::mat4 model = glm::mat4(1.f);
    glm::mat4 view  = glm::mat4(1.f);
    glm::mat4 proj  = glm::mat4(1.f);
};
}  // namespace sui