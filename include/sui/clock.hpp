#pragma once

#include <chrono>

#include "sui/detail/fps.hpp"

namespace sui {
class clock final {
    using std_clock = std::chrono::steady_clock;

  public:
    explicit clock() noexcept = default;

    auto tick() noexcept -> float {
        const auto now        = std::chrono::steady_clock::now();
        const auto epoch_time = now.time_since_epoch();
        const auto next_time =
            std::chrono::duration_cast<std::chrono::duration<float>>(epoch_time).count();
        delta_   = next_time - current_;
        current_ = next_time;

        fps_.update(now);

        return current_;
    }

    [[nodiscard]] auto now() const noexcept -> float { return current_; }

    [[nodiscard]] auto delta_time() const noexcept -> float { return delta_; }

    [[nodiscard]] auto fps() const noexcept -> float { return fps_.get(); }

  private:
    float       current_{};
    float       delta_{};
    detail::fps fps_;
};
}  // namespace sui