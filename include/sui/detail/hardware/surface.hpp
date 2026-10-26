#pragma once

#include <cassert>
#include <expected>
#include <functional>
#include <utility>

#include <hgn_raii/instance.hpp>
#include <hgn_raii/surface.hpp>
#include <kgm/window.hpp>

#include "sui/error.hpp"

namespace sui::detail {
class surface final {
  public:
    [[nodiscard]] static auto try_make(kgm::window& win, hgn::raii::instance& inst) noexcept
        -> std::expected<surface, error> {
        auto native = kgm::try_make_window_surface(inst, win);
        if (not native) return make_error(runtime_error, "Failed to make window surface");
        return surface{hgn::raii::make_surface_khr(std::move(native->get()), inst)};
    }

    [[nodiscard]] auto get() noexcept -> hgn::raii::surface_khr& { return native_; }

    [[nodiscard]] operator hgn::raii::surface_khr&() noexcept { return get(); }

  private:
    explicit surface(hgn::raii::surface_khr&& native) noexcept : native_{std::move(native)} {}

    hgn::raii::surface_khr native_;
};
}  // namespace sui::detail