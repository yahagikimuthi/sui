#pragma once

#include <kgm/kgm.hpp>
#include "sui/detail/fps.hpp"

namespace sui {
class clock final {
  public:
    explicit clock() noexcept = default;

    auto tick() noexcept -> float {
        const auto current = kgm::get_time();
        delta_             = current - last_;
        last_              = current;

        fps_.update();
        return static_cast<float>(current);
    }

    [[nodiscard]] auto delta_time() const noexcept -> float { return static_cast<float>(delta_); }

    [[nodiscard]] auto fps() const noexcept -> float { return fps_.get(); }

  private:
    double      last_{};
    double      delta_{};
    detail::fps fps_;
};
}  // namespace sui