#pragma once

#include <cstdint>
#include <vector>

#include <sui/color.hpp>

#include "use/structs.hpp"

struct mesh final {
    std::vector<vertex>        vertices;
    std::vector<std::uint32_t> indices;
};

[[nodiscard]] inline auto create_floor() noexcept -> mesh {
    return {
        .vertices =
            {vertex{
                 .position = {.x = -50, .y = -10, .z = -50}, .normal = {.y = 1}, .color = sui::white
             },
             vertex{
                 .position = {.x = -50, .y = -10, .z = 50}, .normal = {.y = 1}, .color = sui::red
             },
             vertex{
                 .position = {.x = 50, .y = -10, .z = 50}, .normal = {.y = 1}, .color = sui::green
             },
             vertex{
                 .position = {.x = 50, .y = -10, .z = -50}, .normal = {.y = 1}, .color = sui::blue
             }},
        .indices = {0, 1, 2, 2, 3, 0}
    };
}

[[nodiscard]] inline auto create_box() noexcept -> std::array<mesh, 6> {
    const auto indices = std::vector<std::uint32_t>{0, 1, 2, 2, 3, 0};

    constexpr auto p0 = vertex{.position = {.x = -5, .y = 10, .z = 5}, .color = sui::red};
    constexpr auto p1 = vertex{.position = {.x = 5, .y = 10, .z = 5}, .color = sui::aqua};
    constexpr auto p2 = vertex{.position = {.x = 5, .y = 5, .z = 5}, .color = sui::fuchsia};
    constexpr auto p3 = vertex{.position = {.x = -5, .y = 5, .z = 5}, .color = sui::purple};

    constexpr auto p4 = vertex{.position = p0.position + sui::vec3{.z = 10}, .color = sui::green};
    constexpr auto p5 = vertex{.position = p1.position + sui::vec3{.z = 10}, .color = sui::maroon};
    constexpr auto p6 = vertex{.position = p2.position + sui::vec3{.z = 10}, .color = sui::navy};
    constexpr auto p7 = vertex{.position = p3.position + sui::vec3{.z = 10}, .color = sui::lime};

    const auto front = mesh{
        .vertices =
            {vertex{.position = p0.position, .normal = {.z = -1}, .color = p0.color},
             vertex{.position = p1.position, .normal = {.z = -1}, .color = p1.color},
             vertex{.position = p2.position, .normal = {.z = -1}, .color = p2.color},
             vertex{.position = p3.position, .normal = {.z = -1}, .color = p3.color}},
        .indices = indices
    };

    const auto right = mesh{
        .vertices =
            {vertex{.position = p1.position, .normal = {.x = 1}, .color = p1.color},
             vertex{.position = p5.position, .normal = {.x = 1}, .color = p5.color},
             vertex{.position = p6.position, .normal = {.x = 1}, .color = p6.color},
             vertex{.position = p2.position, .normal = {.x = 1}, .color = p2.color}

            },
        .indices = indices
    };

    const auto left = mesh{
        .vertices =
            {vertex{.position = p4.position, .normal = {.x = -1}, .color = p4.color},
             vertex{.position = p0.position, .normal = {.x = -1}, .color = p0.color},
             vertex{.position = p3.position, .normal = {.x = -1}, .color = p3.color},
             vertex{.position = p7.position, .normal = {.x = -1}, .color = p7.color}},
        .indices = indices
    };

    const auto up = mesh{
        .vertices =
            {vertex{.position = p4.position, .normal = {.y = 1}, .color = p4.color},
             vertex{.position = p5.position, .normal = {.y = 1}, .color = p5.color},
             vertex{.position = p1.position, .normal = {.y = 1}, .color = p1.color},
             vertex{.position = p0.position, .normal = {.y = 1}, .color = p0.color}},
        .indices = indices
    };

    const auto bottom = mesh{
        .vertices =
            {vertex{.position = p7.position, .normal = {.y = -1}, .color = p7.color},
             vertex{.position = p6.position, .normal = {.y = -1}, .color = p6.color},
             vertex{.position = p2.position, .normal = {.y = -1}, .color = p2.color},
             vertex{.position = p3.position, .normal = {.y = -1}, .color = p3.color}},
        .indices = indices
    };

    const auto back = mesh{
        .vertices =
            {vertex{.position = p5.position, .normal = {.z = 1}, .color = p5.color},
             vertex{.position = p4.position, .normal = {.z = 1}, .color = p4.color},
             vertex{.position = p7.position, .normal = {.z = 1}, .color = p7.color},
             vertex{.position = p6.position, .normal = {.z = 1}, .color = p6.color}},
        .indices = indices
    };

    return {front, right, left, up, bottom, back};
}