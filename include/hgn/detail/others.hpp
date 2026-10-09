#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <type_traits>

#include <hgn/types.hpp>

namespace hgn::detail {
template <typename T>
struct is_string_literal final
    : std::bool_constant<
          std::is_array_v<T> and
          std::is_same_v<std::remove_cvref_t<std::remove_extent_t<T>>, char>> {};

template <typename T>
inline constexpr auto is_string_literal_v = is_string_literal<T>::value;

[[nodiscard]] constexpr auto make_version(
    const u32 major, const u32 minor, const u32 patch
) noexcept -> u32 {
    return VK_MAKE_VERSION(major, minor, patch);
}

template <typename T>
struct is_optional_reference final : public std::false_type {};

template <typename T>
struct is_optional_reference<std::optional<T&>> final : public std::true_type {};

template <typename T>
inline constexpr auto is_optional_reference_v = is_optional_reference<T>::value;
}  // namespace hgn::detail