#pragma once

#include <algorithm>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <limits>

#include "sui/extent.hpp"
#include "sui/vec2.hpp"

namespace sui::detail {
class camera final {
  public:
    explicit camera() noexcept = default;

    [[nodiscard]] auto position() const noexcept -> vec2 { return position_; }
    [[nodiscard]] auto zoom() const noexcept -> float { return zoom_; }

    [[nodiscard]] auto calc_proj(const extent virtual_window_size) const noexcept -> glm::mat4 {
        const auto virtual_w = virtual_window_size.width;
        const auto virtual_h = virtual_window_size.height;

        const auto view_w = virtual_w / zoom_;
        const auto view_h = virtual_h / zoom_;

        return glm::ortho(0.f, view_w, 0.f, view_h, -1.f, 1.f);
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