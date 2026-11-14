#pragma once

#include <GLFW/glfw3.h>

namespace kgm::detail {
class terminate_handler final {
  public:
    terminate_handler() noexcept { ++instance_cnt_; }

    terminate_handler([[maybe_unused]] const terminate_handler& other) noexcept { ++instance_cnt_; }

    auto operator=(const terminate_handler&) noexcept -> terminate_handler& = default;

    terminate_handler([[maybe_unused]] terminate_handler&& other) noexcept { ++instance_cnt_; }

    auto operator=(terminate_handler&&) noexcept -> terminate_handler& = default;

    ~terminate_handler() noexcept {
        --instance_cnt_;
        if (instance_cnt_ == 0) glfwTerminate();
    }

  private:
    static inline constinit int instance_cnt_;
};
}  // namespace kgm::detail