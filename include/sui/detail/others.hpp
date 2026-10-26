#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include <hgn/enums.hpp>

namespace sui::detail {
static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);

constexpr void nothing([[maybe_unused]] auto&&... _) noexcept {}

template <typename F>
    requires std::is_nothrow_invocable_r_v<void, F>
class scope_exit final {
  public:
    [[nodiscard]] explicit constexpr scope_exit(F&& f) noexcept
        requires std::is_nothrow_move_constructible_v<F>
        : f_{std::move(f)} {}
    [[nodiscard]] explicit constexpr scope_exit(const F& f) noexcept
        requires std::is_nothrow_copy_constructible_v<F>
        : f_{f} {}

    constexpr scope_exit(const scope_exit&)                              = delete;
    constexpr auto operator=(const scope_exit&) noexcept -> scope_exit&  = delete;
    constexpr scope_exit(scope_exit&& other) noexcept                    = delete;
    constexpr auto operator=(scope_exit&& other) noexcept -> scope_exit& = delete;

    constexpr ~scope_exit() noexcept {
        if (is_valid_) std::invoke(f_);
    }

    [[nodiscard]] constexpr auto is_valid() const noexcept -> bool { return is_valid_; }

    constexpr void active() noexcept { is_valid_ = false; }

    constexpr void release() noexcept { is_valid_ = false; }

  private:
    F    f_;
    bool is_valid_{true};
};

[[nodiscard]] inline auto read_file(const std::filesystem::path& path) noexcept
    -> std::optional<std::vector<char>> {
    auto file = std::ifstream{path, std::ios::ate | std::ios::binary};

    if (not file.is_open()) return std::nullopt;

    const auto size   = static_cast<std::int64_t>(file.tellg());
    auto       buffer = std::vector<char>(static_cast<std::size_t>(size));

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

template <typename T>
[[nodiscard]] constexpr auto to_optional(const std::expected<T, hgn::result>& value) noexcept
    -> std::optional<T> {
    auto out = std::optional<T>{std::nullopt};
    if (not value) return out;
    return out.emplace(*value);
}

template <typename T>
[[nodiscard]] constexpr auto to_optional(std::expected<T, hgn::result>&& rvalue) noexcept
    -> std::optional<T> {
    auto out = std::optional<T>{std::nullopt};
    if (not rvalue) return out;
    return std::move(out.emplace(std::move(*rvalue)));
}

template <typename T>
[[nodiscard]] constexpr auto to_optional(
    const std::expected<std::reference_wrapper<T>, hgn::result>& reference
) noexcept -> std::optional<T&> {
    auto out = std::optional<T&>{std::nullopt};
    if (not reference) return out;
    return out.emplace(*reference);
}
}  // namespace sui::detail