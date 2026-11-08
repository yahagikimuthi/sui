#pragma once

#include <chrono>
namespace sui::detail {
class fps final {
    using clock      = std::chrono::steady_clock;
    using time_point = clock::time_point;
    using duration   = std::chrono::duration<float>;

  public:
    explicit fps() noexcept = default;

    void update() noexcept {
        const auto now = clock::now();
        delta_time_    = std::chrono::duration_cast<duration>(now - last_time_).count();
        last_time_     = now;

        ++frame_count_;

        const auto time_since_fps_update =
            std::chrono::duration_cast<duration>(now - fps_last_updated_).count();

        if (time_since_fps_update < 1.f) return;
        current_fps_      = static_cast<float>(frame_count_) / time_since_fps_update;
        frame_count_      = 0;
        fps_last_updated_ = now;
    }

    [[nodiscard]] auto get() const noexcept -> float { return current_fps_; }

  private:
    time_point    last_time_{clock::now()};
    time_point    fps_last_updated_{last_time_};
    float         delta_time_{};
    float         current_fps_{};
    std::uint32_t frame_count_{};
};
}  // namespace sui::detail