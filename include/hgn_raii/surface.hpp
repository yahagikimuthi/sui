#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <hgn/instance.hpp>
#include <hgn/surface.hpp>

namespace hgn::raii {
class surface_khr final {
  public:
    explicit surface_khr(hgn::surface_khr&& native, hgn::instance& inst) noexcept
        : native_{native}, instance_{inst} {}

    surface_khr(const surface_khr&) noexcept                    = delete;
    auto operator=(const surface_khr&) noexcept -> surface_khr& = delete;

    surface_khr(surface_khr&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, instance_{other.instance_} {}

    auto operator=(surface_khr&& other) noexcept -> surface_khr& {
        if (this == &other) return *this;

        destroy();

        native_   = std::exchange(other.native_, std::nullopt);
        instance_ = other.instance_;

        return *this;
    }

    ~surface_khr() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::surface_khr& { return *native_; }

    [[nodiscard]] operator hgn::surface_khr&() noexcept { return get(); }

  private:
    void destroy() noexcept { hgn::destroy_surface_khr(instance_, get()); }

    std::optional<hgn::surface_khr&>      native_;
    std::reference_wrapper<hgn::instance> instance_;
};
}  // namespace hgn::raii