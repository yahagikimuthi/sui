#pragma once

#include <chrono>

#include "kgm/detail/fps.hpp"

namespace sui {
class clock final {
  public:
    explicit clock() noexcept
        : init_epoch_{std::chrono::duration_cast<std::chrono::duration<double>>(
                          std::chrono::steady_clock::now().time_since_epoch()
          )
                          .count()} {}

    auto tick() noexcept -> float {
        const auto now = std::chrono::steady_clock::now();
        const auto current_epoch =
            std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch())
                .count();

        const auto time_from_init = static_cast<float>(current_epoch - init_epoch_);

        delta_   = time_from_init - current_;
        current_ = time_from_init;

        fps_.update(now);

        return current_;
    }

    [[nodiscard]] auto now() const noexcept -> float { return current_; }

    [[nodiscard]] auto delta_time() const noexcept -> float { return delta_; }

    [[nodiscard]] auto fps() const noexcept -> float { return fps_.get(); }

  private:
    double      init_epoch_;
    float       current_{};
    float       delta_{};
    detail::fps fps_;
};
}  // namespace sui