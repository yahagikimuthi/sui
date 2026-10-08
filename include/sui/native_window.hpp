#pragma once

#include <expected>
#include <kgm/kgm.hpp>
#include <kgm/window.hpp>
#include <optional>
#include <string_view>
#include <utility>

#include "sui/error.hpp"
#include "sui/types.hpp"

namespace sui {
class native_window final {
  public:
    [[nodiscard]] static auto try_make(
        const u32 width, const u32 height, std::string_view title = "No Title"
    ) noexcept -> std::expected<native_window, error> {
        kgm::window_hint(kgm::client_api, kgm::no_api);
        auto win = kgm::create_window(width, height, title);
        if (not win) return make_error(runtime_error, "Failed to create window.\n");

        return native_window{*win};
    }

    native_window(const native_window&) noexcept                    = delete;
    auto operator=(const native_window&) noexcept -> native_window& = delete;

    native_window(native_window&& other) noexcept
        : window_{std::exchange(other.window_, std::nullopt)} {}
    auto operator=(native_window&& other) noexcept -> native_window& {
        if (this == &other) return *this;
        destroy();

        window_ = std::exchange(other.window_, std::nullopt);

        return *this;
    }

    ~native_window() noexcept { destroy(); }

    [[nodiscard]] auto is_open() const noexcept -> bool {
        if (not window_) return false;
        return kgm::window_should_close(*window_);
    }

  private:
    explicit native_window(kgm::window& win) : window_{win} {}

    void destroy() noexcept {
        if (not window_) return;
        kgm::destroy_window(*window_);
    }

    std::optional<kgm::window&> window_;
};
}  // namespace sui