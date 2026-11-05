#pragma once

#include <string_view>

#include "sui/detail/debug.hpp"

namespace sui {
inline void set_debug_fn(void (&fn)(std::string_view)) noexcept {
    detail::debugger::set_fn(fn);
}

inline void set_default_debug_fn() noexcept {
    detail::debugger::set_default();
}

inline void reset_debug_fn() noexcept {
    detail::debugger::reset();
}
}  // namespace sui