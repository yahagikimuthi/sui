#pragma once

#include <cassert>
#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/instance.hpp>
#include <hgn/surface.hpp>
#include <kgm/window.hpp>

#include "sui/error.hpp"

namespace sui {
class surface final {
  public:
    [[nodiscard]] static auto try_make(kgm::window& win, hgn::instance& inst) noexcept
        -> std::expected<surface, error> {
        auto native = kgm::try_make_window_surface(inst, win);
        if (not native) return make_error(runtime_error, "Failed to make window surface");
        return surface{native->get(), inst};
    }

    surface(const surface&) noexcept = delete;

    auto operator=(const surface&) noexcept -> surface& = delete;

    surface(surface&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, instance_{other.instance_} {}

    auto operator=(surface&& other) noexcept -> surface& {
        if (this == &other) return *this;

        destroy();

        native_   = std::exchange(other.native_, std::nullopt);
        instance_ = other.instance_;

        return *this;
    }

    ~surface() noexcept { destroy(); }

    [[nodiscard]] auto native() noexcept -> hgn::surface_khr& { return *native_; }

  private:
    explicit surface(hgn::surface_khr& native, hgn::instance& inst) noexcept
        : native_{native}, instance_{inst} {}

    void destroy() noexcept {
        if (not native_) return;
        hgn::destroy_surface_khr(instance_.get(), *native_);
    }

    std::optional<hgn::surface_khr&>      native_;
    std::reference_wrapper<hgn::instance> instance_;
};
}  // namespace sui