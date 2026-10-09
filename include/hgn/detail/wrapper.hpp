#pragma once

#include <optional>
#include <type_traits>
#include <variant>

#include "hgn/types.hpp"

namespace hgn::detail {
template <typename Native>
    requires std::is_class_v<Native>
class wrapper {
  public:
    explicit wrapper() noexcept
        requires std::is_default_constructible_v<Native>
    = default;
    explicit wrapper(Native& native) noexcept : native_{std::optional<Native&>{native}} {}

    ~wrapper() noexcept = default;

    [[nodiscard]] auto native() noexcept -> Native& {
        return native_.visit(
            overloaded{
                [](std::optional<Native&> ref) noexcept -> Native& {
                    auto& out = *ref;
                    return out;
                },
                [](Native& native) noexcept -> Native& { return native; }
            }
        );
    }

    [[nodiscard]] auto native() const noexcept -> const Native& {
        return native_.visit(
            overloaded{
                [](const std::optional<Native&> ref) noexcept -> const Native& {
                    const auto& out = *ref;
                    return out;
                },
                [](const Native& native) noexcept -> const Native& { return native; }
            }
        );
    }

  protected:
    wrapper(const wrapper&) noexcept                    = default;
    auto operator=(const wrapper&) noexcept -> wrapper& = default;
    wrapper(wrapper&&) noexcept                         = default;
    auto operator=(wrapper&&) noexcept -> wrapper&      = default;

  private:
    std::variant<std::optional<Native&>, Native> native_{Native{}};
};
}  // namespace hgn::detail

namespace hgn {
template <typename Native>
    requires std::is_class_v<Native>
class view final : public detail::wrapper<Native> {};

template <typename T>
struct is_view final : std::false_type {};

template <typename T>
struct is_view<view<T>> : std::true_type {};

template <typename T>
inline constexpr auto is_view_v = is_view<T>::value;
}  // namespace hgn