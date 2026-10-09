#pragma once

#include <optional>
#include <variant>

#include "hgn/types.hpp"

namespace hgn::detail {

// 完全型のポインタをラップしたい場合は使用する
template <typename Native>
    requires std::is_class_v<Native>
class wrapper {
  public:
    explicit wrapper() noexcept
        requires std::is_default_constructible_v<Native>
    = default;
    explicit wrapper(Native& native) noexcept : native_{std::optional<Native&>{native}} {}

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
    std::variant<std::optional<Native&>, Native> native_{Native{}};
};
}  // namespace hgn::detail