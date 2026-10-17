#pragma once

#include <expected>
#include <optional>
#include <string_view>
#include <utility>

#include <kgm/kgm.hpp>
#include <kgm/window.hpp>

#include "sui/error.hpp"

namespace sui {
class native_window final {
  public:
    [[nodiscard]] static auto try_make(
        const std::uint32_t width, const std::uint32_t height, std::string_view title = "No Title"
    ) noexcept -> std::expected<native_window, error> {
        if (not kgm::try_init()) return make_error(runtime_error, "Failed to initialize Kagami");

        kgm::set_window_hint(kgm::client_api, kgm::no_api);
        auto win = kgm::try_make_window(width, height, title);
        if (not win) return make_error(runtime_error, "Failed to make window");

        return native_window{*win};
    }

    native_window(const native_window&) noexcept = delete;

    auto operator=(const native_window&) noexcept -> native_window& = delete;

    native_window(native_window&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)} {
        if (native_) kgm::set_window_user(*native_, *this);
    }

    auto operator=(native_window&& other) noexcept -> native_window& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        if (native_) kgm::set_window_user(*native_, *this);

        return *this;
    }

    ~native_window() noexcept { destroy(); }

    [[nodiscard]] auto is_open() const noexcept -> bool {
        if (not native_) return false;
        return not kgm::window_should_close(*native_);
    }

    [[nodiscard]] auto native() noexcept -> kgm::window& { return *native_; }

  private:
    explicit native_window(kgm::window& native) : native_{native} {
        kgm::set_window_user(native, *this);
    }

    void destroy() noexcept {
        if (not native_) return;
        kgm::destroy_window(*native_);
        kgm::terminate();
    }

    std::optional<kgm::window&> native_;
};
}  // namespace sui