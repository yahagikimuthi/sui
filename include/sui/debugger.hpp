#pragma once

#include <optional>
#include <string_view>

namespace sui {
class debugger final {
  public:
    static void set(void (&set_func)(const std::string_view)) noexcept { func = set_func; }

    static void notify(const std::string_view message) noexcept {
        if (func) (*func)(message);
    }

  private:
    static inline constinit auto func =
        std::optional<void (&)(const std::string_view)>{std::nullopt};
};

inline void set_debug_function(void (&set_func)(const std::string_view)) noexcept {
    debugger::set(set_func);
}
}  // namespace sui