#pragma once

#include <functional>
#include <iostream>
#include <string_view>
#include <utility>
#include <variant>

namespace sui {
class debugger final {
    using r_arg_t = void(std::string_view);
    using fn_t    = std::variant<std::reference_wrapper<r_arg_t>, std::function<r_arg_t>>;

  public:
    static void set(void (&fn)(std::string_view)) noexcept { fn_ = std::ref(fn); }

    template <typename F>
        requires std::is_invocable_r_v<void, F, const std::string_view>
    static void set(F&& f) noexcept {
        fn_ = std::forward<F>(f);
    }

    static void reset() noexcept { set(null_fn_); }

    static void notify(const std::string_view message) noexcept {
        fn_.visit([message](auto&& fn) noexcept -> void { fn(message); });
    }

  private:
    static constexpr auto null_lambda_ =
        []([[maybe_unused]] std::string_view message) constexpr noexcept -> void {};
    static constexpr auto* const null_fn_ptr_ = static_cast<r_arg_t*>(null_lambda_);
    static constexpr auto&       null_fn_     = *null_fn_ptr_;
    static inline constinit auto fn_          = fn_t{std::ref(null_fn_)};  // NOLINT
};

inline void set_debug_function(void (&fn)(std::string_view)) noexcept {
    debugger::set(fn);
}

template <typename F>
    requires std::is_invocable_r_v<void, F, const std::string_view>
inline void set_debug_function(F&& fn) noexcept {
    debugger::set(std::forward<F>(fn));
}

inline void reset_debug_function() noexcept {
    debugger::reset();
}
}  // namespace sui