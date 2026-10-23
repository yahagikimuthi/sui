#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

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
    requires std::is_lvalue_reference_v<T>
class option final {
    using value_type = std::remove_reference_t<T>;
    using pointer    = value_type*;

  public:
    constexpr option() noexcept = default;

    constexpr option(T value) noexcept  // NOLINT
        :
#if __cpp_lib_optional >= 202506L
          m_{value}
#else
          m_{&value}
#endif
    {
    }

    [[nodiscard]] explicit operator bool() noexcept {
#if __cpp_lib_optional >= 202506L
        return m_.has_value();
#else
        return m_ != nullptr;
#endif
    }

    [[nodiscard]] auto operator*() noexcept -> value_type& { return *m_; }

    [[nodiscard]] auto operator->() noexcept -> pointer {
#if __cpp_lib_optional >= 202506L
        auto& value = *m_;
        return *value;
#else
        return m_;
#endif
    }

  private:
#if __cpp_lib_optional >= 202506L
    std::optional<value_type&> m_{std::nullopt};
#else
    pointer m_{nullptr};
#endif
};
}  // namespace sui::detail