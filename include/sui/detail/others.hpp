#pragma once

#include <type_traits>

namespace sui::detail {
template <typename T>
struct is_string_literal final
    : std::bool_constant<
          std::is_array_v<T> and
          std::is_same_v<std::remove_cvref_t<std::remove_extent_t<T>>, char>> {};

template <typename T>
inline constexpr auto is_string_literal_v = is_string_literal<T>::value;

static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);

constexpr auto nothing([[maybe_unused]] auto&&... _) noexcept {}
}  // namespace sui::detail