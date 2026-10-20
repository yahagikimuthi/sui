#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string_view>
#include <utility>

#include "sui/error.hpp"
#include "sui/native_window.hpp"
#include "sui/vk_context.hpp"

namespace sui::detail {

class window_impl final {
  public:
    [[nodiscard]] static auto try_make(
        const std::uint32_t width, const std::uint32_t height, const std::string_view title
    ) noexcept -> std::expected<window_impl, error> {
        auto win_res = native_window::try_make(width, height, title);
        if (not win_res)
            return make_error(runtime_error, "Failed to make native window.", win_res.error());

        auto ctx_res = vk_context::try_make(win_res->get());
        if (not ctx_res)
            return make_error(runtime_error, "Failed to make vulkan context.", ctx_res.error());

        return window_impl{std::move(*win_res), std::move(*ctx_res)};
    }

    [[nodiscard]] auto try_begin_rendering() noexcept -> bool { return ctx_.try_begin_rendering(); }

    void rendering(const std::span<const float> vertices) noexcept { ctx_.try_rendering(vertices); }

    [[nodiscard]] auto try_end_rendering() noexcept -> bool { return ctx_.try_end_rendering(); }

  private:
    explicit window_impl(native_window&& win, vk_context&& ctx) noexcept
        : win_{std::move(win)}, ctx_{std::move(ctx)} {}

    native_window win_;
    vk_context    ctx_;
};
}  // namespace sui::detail