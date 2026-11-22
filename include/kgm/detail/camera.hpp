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

    [[nodiscard]] auto target() const noexcept -> const glm::vec3& { return target_; }
    [[nodiscard]] auto target() noexcept -> glm::vec3& { return target_; }

    [[nodiscard]] auto front() const noexcept -> glm::vec3 {
        auto f = glm::vec3{};
        f[0]   = std::cos(yaw_);
        f[1]   = 0.f;
        f[2]   = std::sin(yaw_);
        return glm::normalize(f);
    }

    [[nodiscard]] auto calc_view() const noexcept -> glm::mat4 {
        return glm::lookAt(
            static_cast<glm::vec3>(eye_), static_cast<glm::vec3>(target_), glm::vec3{0.f, 1.f, 0.f}
        );
    }

  private:
    glm::vec3 eye_{0.f, 0.f, 5.f};
    glm::vec3 target_{};
    float     yaw_{glm::radians(-90.f)};
};
}  // namespace sui