#pragma once

#include <GLFW/glfw3.h>

namespace kgm::detail {
class terminate_handler final {
  public:
    explicit terminate_handler() noexcept { ++instance_cnt_; }

    terminate_handler([[maybe_unused]] const terminate_handler& other) noexcept { ++instance_cnt_; }

    auto operator=(const terminate_handler&) noexcept -> terminate_handler& = default;

    terminate_handler(terminate_handler&&) noexcept = default;

    auto operator=([[maybe_unused]] terminate_handler&& other) noexcept -> terminate_handler& {
        --instance_cnt_;
        return *this;
    }

    ~terminate_handler() noexcept {
        if (instance_cnt_ == 0) glfwTerminate();
    }

  private:
    static inline constinit int instance_cnt_;
};
}  // namespace kgm::detail