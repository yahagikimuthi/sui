#pragma once

#include <expected>
#include <kgm/kgm.hpp>
#include <string_view>

#include "kgm/enums.hpp"
#include "kgm/window.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class window final {
  public:
    [[nodiscard]] static auto make(
        const std::uint32_t width, const std::uint32_t height, const std::string_view title
    ) noexcept -> std::expected<window, error> {
        auto raw = kgm::get_window_creator<window>()
                       .hint(kgm::window_hint::client_api, kgm::window_value::no_api)
                       .extent(width, height)
                       .title(title)
                       .create();

        if (not raw) return make_error(runtime_error, "Failed to make window.");

        return window{std::move(*raw)};
    }

    [[nodiscard]] auto get() noexcept -> kgm::window<window>& { return raw_; }

  private:
    explicit window(kgm::window<window>&& raw) noexcept : raw_{std::move(raw)} {}

    kgm::window<window> raw_;
};
}  // namespace sui::detail