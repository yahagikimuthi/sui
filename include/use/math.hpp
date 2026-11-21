#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>

struct transform final {
    glm::vec3 position;
    glm::vec3 rotation;  // (pitch, yaw, roll)
    glm::vec3 scale;
};

[[nodiscard]] constexpr auto calc_model(const transform& t) noexcept -> glm::mat4 {
    auto m = glm::mat4{1.f};

    m = glm::translate(m, t.position);

    if (t.rotation[0] != 0.f) m = glm::rotate(m, t.rotation[0], glm::vec3{1.f, 0.f, 0.f});
    if (t.rotation[1] != 0.f) m = glm::rotate(m, t.rotation[1], glm::vec3{0.f, 1.f, 0.f});
    if (t.rotation[2] != 0.f) m = glm::rotate(m, t.rotation[2], glm::vec3{0.f, 0.f, 1.f});

    m = glm::scale(m, t.scale);

    return m;
}