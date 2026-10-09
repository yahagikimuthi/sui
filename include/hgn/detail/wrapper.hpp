#pragma once

#include <optional>
#include <variant>

#include "hgn/types.hpp"

namespace hgn::detail {
// 不完全型をラップする需要はそもそも存在しない
template <typename Native>
    requires(not std::is_reference_v<Native>) and (not std::is_pointer_v<Native>)
class wrapper {
  public:
    explicit wrapper() noexcept
        requires std::is_default_constructible_v<Native>
        : native_{Native{}} {}
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
    std::variant<std::optional<Native&>, Native> native_;
};
}  // namespace hgn::detail