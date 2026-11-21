#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "sui/vec.hpp"

namespace sui::detail {
class camera final {
  public:
    camera() noexcept = default;

    auto eye(const vec3& v) noexcept -> camera& {
        eye_ = v;
        return *this;
    }

    auto target(const vec3& v) noexcept -> camera& {
        target_ = v;
        return *this;
    }

    auto up(const vec3& v) noexcept -> camera& {
        up_ = v;
        return *this;
    }

    [[nodiscard]] auto calc_view() const noexcept -> glm::mat4 {
        return glm::lookAt(
            static_cast<glm::vec3>(eye_),
            static_cast<glm::vec3>(target_),
            static_cast<glm::vec3>(up_)
        );
    }

  private:
    vec3 eye_{};
    vec3 target_{};
    vec3 up_{};
};
}  // namespace sui::detail