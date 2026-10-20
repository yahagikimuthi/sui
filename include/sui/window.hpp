#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

#include "sui/detail/window_impl.hpp"
#include "sui/error.hpp"

namespace sui {
class window final {
  public:
    [[nodiscard]] static auto try_make(
        const std::uint32_t    width,
        const std::uint32_t    height,
        const std::string_view title = "No Title"
    ) noexcept -> std::expected<window, error>;

    [[nodiscard]] auto is_open() const noexcept -> bool;

    [[nodiscard]] auto try_begin_rendering() noexcept -> bool;

    void rendering(const std::span<const float> vertices) noexcept;

    [[nodiscard]] auto try_end_rendering() noexcept -> bool;

  private:
    explicit window(detail::window_impl&& impl) noexcept;

    detail::window_impl impl_;
};

[[nodiscard]] inline auto try_make_window(
    const std::uint32_t width, const std::uint32_t height, const std::string_view title = "No Title"
) noexcept -> std::expected<window, error>;
}  // namespace sui

// ==================================================
// Implementation details only below here
// ==================================================

namespace sui {
[[nodiscard]] inline auto window::try_make(
    const std::uint32_t width, const std::uint32_t height, const std::string_view title
) noexcept -> std::expected<window, error> {
    auto res = detail::window_impl::try_make(width, height, title);
    if (not res) return std::unexpected{res.error()};
    return window{std::move(*res)};
}

[[nodiscard]] inline auto window::is_open() const noexcept -> bool {
    return impl_.is_open();
}

[[nodiscard]] inline auto window::try_begin_rendering() noexcept -> bool {
    return impl_.try_begin_rendering();
}

inline void window::rendering(const std::span<const float> vertices) noexcept {
    impl_.rendering(vertices);
}

[[nodiscard]] inline auto window::try_end_rendering() noexcept -> bool {
    return impl_.try_end_rendering();
}

inline window::window(detail::window_impl&& impl) noexcept : impl_{std::move(impl)} {}

[[nodiscard]] inline auto try_make_window(
    const std::uint32_t width, const std::uint32_t height, const std::string_view title
) noexcept -> std::expected<window, error> {
    return window::try_make(width, height, title);
}
}  // namespace sui