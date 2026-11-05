#pragma once

#include <iostream>
#include <string_view>

namespace sui::detail {
class debugger final {
  public:
    static void set_fn(void (*fn)(std::string_view)) noexcept { fn_ = fn; }

    static void set_default() noexcept { is_default_ = true; }

    static void reset() noexcept { fn_ = nullptr; }

    static void notify(const std::string_view message) noexcept {
        if (fn_ != nullptr)
            fn_(message);
        else if (is_default_)
            std::cerr << message;
    }

  private:
    static constinit inline auto* fn_         = static_cast<void (*)(std::string_view)>(nullptr);
    static constinit inline auto  is_default_ = false;
};
}  // namespace sui::detail