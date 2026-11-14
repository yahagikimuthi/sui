#pragma once

#include <chrono>

namespace sui::detail {
class fps final {
  public:
    fps() noexcept = default;

    void update(const std::chrono::steady_clock::time_point& now) noexcept {
        ++frame_count_;

        const auto time_since_update =
            std::chrono::duration_cast<std::chrono::duration<float>>(now - last_updated_).count();

        if (time_since_update < 1.f) return;

        current_      = static_cast<float>(frame_count_) / time_since_update;
        frame_count_  = 0;
        last_updated_ = now;
    }

    [[nodiscard]] auto get() const noexcept -> float { return current_; }

  private:
    std::chrono::steady_clock::time_point last_updated_{std::chrono::steady_clock::now()};
    float                                 current_{};
    std::uint32_t                         frame_count_{};
};
}  // namespace sui::detail