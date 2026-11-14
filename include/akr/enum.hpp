#pragma once

#include <cstdint>

namespace akr {
enum class error_category : std::uint8_t { runtime, logic, unknown };

inline constexpr auto runtime_error = error_category::runtime;
inline constexpr auto logic_error   = error_category::logic;
inline constexpr auto unknown_error = error_category::unknown;
}  // namespace akr