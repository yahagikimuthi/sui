#pragma once

#include <cassert>
#include <optional>
#include <utility>

#include <hgn/instance.hpp>
#include <hgn/surface.hpp>

namespace sui {
class surface final {
  public:
    explicit surface(hgn::surface_khr& native, hgn::instance& inst) noexcept
        : native_{native}, instance_{inst} {}

    surface(const surface&) noexcept = delete;

    auto operator=(const surface&) noexcept -> surface& = delete;

    surface(surface&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)},
          instance_{std::exchange(other.instance_, std::nullopt)} {}

    auto operator=(surface&& other) noexcept -> surface& {
        if (this == &other) return *this;

        destroy();

        native_   = std::exchange(other.native_, std::nullopt);
        instance_ = std::exchange(other.instance_, std::nullopt);

        return *this;
    }

    ~surface() noexcept { destroy(); }

    [[nodiscard]] auto native() noexcept -> hgn::surface_khr& { return native_.value(); }

  private:
    void destroy() noexcept {
        if (not native_) return;
        assert(instance_);
        hgn::destroy_surface_khr(*instance_, *native_);
    }

    std::optional<hgn::surface_khr&> native_;
    std::optional<hgn::instance&>    instance_;
};
}  // namespace sui