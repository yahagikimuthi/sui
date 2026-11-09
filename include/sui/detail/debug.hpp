#pragma once

#include <functional>
#include <iostream>
#include <string_view>
#include <utility>

namespace sui::detail {
class debugger final {
  public:
    static void set_fn(std::function<void(std::string_view)>&& fn) noexcept { fn_ = std::move(fn); }

    static void set_default() noexcept { is_default_ = true; }

    static void reset() noexcept { fn_ = std::function<void(std::string_view)>{}; }

    static void notify(const std::string_view message) noexcept {
        if (fn_)
            fn_(message);
        else if (is_default_)
            std::cerr << message;
    }

  private:
    static inline auto           fn_         = std::function<void(std::string_view)>{};  // NOLINT
    static constinit inline auto is_default_ = false;
};

[[nodiscard]] inline auto fail(const std::string_view message) noexcept -> bool {
    debugger::notify(message);
    return false;
}
}  // namespace sui::detail