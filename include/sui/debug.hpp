#pragma once

#include <string_view>
#include <type_traits>
#include <utility>

#include "sui/detail/debug_impl.hpp"

namespace sui {
inline void set_debug_function(void (&fn)(std::string_view)) noexcept {
    detail::debugger::set(fn);
}

template <typename F>
    requires std::is_invocable_r_v<void, F, const std::string_view>
inline void set_debug_function(F&& fn) noexcept {
    detail::debugger::set(std::forward<F>(fn));
}

inline void reset_debug_function() noexcept {
    detail::debugger::reset();
}
}  // namespace sui