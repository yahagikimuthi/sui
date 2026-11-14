#pragma once

#include <algorithm>
#include <limits>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "sui/vec2.hpp"

namespace sui::detail {
class camera final {
  public:
    camera() noexcept = default;

    [[nodiscard]] auto position() const noexcept -> vec2 { return position_; }
    [[nodiscard]] auto zoom() const noexcept -> float { return zoom_; }

    [[nodiscard]] auto calc_proj() const noexcept -> glm::mat4 {
        return glm::ortho(
            -800.f / 600.f * zoom_, 800.f / 600.f * zoom_, -zoom_, zoom_, -1.0f, 1.0f
        );
    }

    [[nodiscard]] auto calc_view() const noexcept -> glm::mat4 {
        return glm::translate(glm::mat4(1.f), static_cast<glm::vec3>(-position_));
    }

    void position(const vec2 pos) noexcept { position_ = pos; }

    void update_from_zoom(const float factor, const vec2 reference_point) noexcept {
        constexpr auto epsilon  = std::numeric_limits<float>::epsilon();
        const auto     current  = zoom_ < epsilon ? epsilon : zoom_;
        const auto     new_zoom = std::clamp(current * factor, 0.05f, 10.f);

        const auto ref_pos       = static_cast<glm::vec2>(reference_point);
        const auto world_ref_pos = static_cast<glm::vec2>(position_) + (ref_pos / current);

        const auto new_pos = world_ref_pos - (ref_pos / new_zoom);

        position_ = {.x = new_pos[0], .y = new_pos[1]};
        zoom_     = new_zoom;
    }

  private:
    vec2  position_{};
    float zoom_{1.f};
};
}  // namespace sui::detail