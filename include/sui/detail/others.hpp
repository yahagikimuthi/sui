#pragma once

#include <functional>
#include <type_traits>
#include <utility>

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

constexpr void nothing([[maybe_unused]] auto&&... _) noexcept {}

template <typename F>
    requires std::is_nothrow_invocable_r_v<void, F>
class scope_exit final {
  public:
    [[nodiscard]] explicit scope_exit(F&& f) noexcept : f_{std::move(f)} {}
    [[nodiscard]] explicit scope_exit(const F& f) noexcept : f_{f} {}

    scope_exit(const scope_exit&) = delete;

    auto operator=(const scope_exit&) noexcept -> scope_exit& = delete;

    scope_exit(scope_exit&& other) noexcept
        : f_{std::move(other.f_)}, is_valid_{std::exchange(other.is_valid_, false)} {}

    auto operator=(scope_exit&& other) noexcept -> scope_exit& {
        if (this == &other) return *this;

        f_        = std::move(other.f_);
        is_valid_ = std::exchange(other.is_valid_, false);

        return *this;
    }

    ~scope_exit() noexcept {
        if (is_valid_) std::invoke(f_);
    }

    [[nodiscard]] auto is_valid() const noexcept -> bool { return is_valid_; }

    void active() noexcept { is_valid_ = false; }

    void release() noexcept { is_valid_ = false; }

  private:
    F    f_;
    bool is_valid_{true};
};
}  // namespace sui::detail