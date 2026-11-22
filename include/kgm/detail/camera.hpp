#pragma once

#include <cmath>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace sui {
class camera final {
  public:
    camera() noexcept = default;

    [[nodiscard]] auto eye() noexcept -> glm::vec3& { return eye_; }
    [[nodiscard]] auto eye() const noexcept -> const glm::vec3& { return eye_; }

    [[nodiscard]] auto yaw_deg() noexcept -> float& { return yaw_deg_; }
    [[nodiscard]] auto yaw_deg() const noexcept -> const float& { return yaw_deg_; }

    [[nodiscard]] auto pitch_deg() noexcept -> float& { return pitch_deg_; }
    [[nodiscard]] auto pitch_deg() const noexcept -> const float& { return pitch_deg_; }

    [[nodiscard]] auto front() const noexcept -> glm::vec3 {
        const auto yaw   = glm::radians(yaw_deg_);
        const auto pitch = glm::radians(pitch_deg_);
        auto       f     = glm::vec3{};
        f[0]             = std::cos(yaw) * std::cos(pitch);
        f[1]             = std::sin(pitch);
        f[2]             = std::sin(yaw) * std::cos(pitch);
        return glm::normalize(f);
    }

    [[nodiscard]] auto calc_view() const noexcept -> glm::mat4 {
        return glm::lookAt(static_cast<glm::vec3>(eye_), eye_ + front(), glm::vec3{0.f, 1.f, 0.f});
    }

  private:
    glm::vec3 eye_{0.f, 0.f, 5.f};
    float     yaw_deg_{-90.f};
    float     pitch_deg_{};
};
}  // namespace sui