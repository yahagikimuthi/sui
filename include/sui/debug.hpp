#pragma once

#include <functional>
#include <string_view>
#include <utility>

#include "sui/detail/others.hpp"

namespace sui {
inline void set_debug_fn(std::function<void(std::string_view)> fn) noexcept {
    detail::debugger::set_fn(std::move(fn));
}

inline void set_default_debug_fn() noexcept {
    detail::debugger::set_default();
}

inline void reset_debug_fn() noexcept {
    detail::debugger::reset();
}
}  // namespace sui