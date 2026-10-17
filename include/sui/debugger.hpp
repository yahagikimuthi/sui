#pragma once

#include <functional>
#include <string_view>
#include <utility>
#include <variant>

namespace sui {
class debugger final {
    using fn_ptr_t = void (*)(const std::string_view);
    using fn_ref_t = std::reference_wrapper<std::remove_pointer_t<fn_ptr_t>>;

    using fn_t = std::variant<fn_ref_t, std::function<void(const std::string_view)>>;

  public:
    static void set(void (&fn)(const std::string_view)) noexcept { fn_ = fn_ref_t{fn}; }

    template <typename F>
        requires std::is_invocable_r_v<void, F, const std::string_view>
    static void set(F&& f) noexcept {
        fn_ = std::function<void(const std::string_view)>{std::forward<F>(f)};
    }

    static void reset() noexcept { set(*default_fn_); }

    static void notify(const std::string_view message) noexcept {
        fn_.visit([message](auto&& fn) noexcept -> void { fn(message); });
    }

  private:
    static constexpr auto* default_fn_ =
        fn_ptr_t{[]([[maybe_unused]] const std::string_view message) constexpr noexcept -> void {}};

    static inline constinit auto fn_ = fn_t{fn_ref_t{*default_fn_}};  // NOLINT
};

inline void set_debug_function(void (&fn)(const std::string_view)) noexcept {
    debugger::set(fn);
}

template <typename F>
    requires std::is_invocable_r_v<void, F, const std::string_view>
inline void set(F&& fn) noexcept {
    debugger::set(std::forward<F>(fn));
}

inline void reset_debug_function() noexcept {
    debugger::reset();
}
}  // namespace sui