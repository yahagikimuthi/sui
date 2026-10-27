#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <hgn/declaration.hpp>
#include <hgn/surface.hpp>

namespace hgn::raii {
class surface_khr final {
  public:
    [[nodiscard]] static auto make(hgn::surface_khr&& native, hgn::instance& inst) noexcept
        -> raii::surface_khr {
        return raii::surface_khr{std::move(native), inst};
    }

    explicit surface_khr(hgn::surface_khr&& native, hgn::instance& inst) noexcept
        : native_{native}, instance_{inst} {}

    surface_khr(const raii::surface_khr&) noexcept                          = delete;
    auto operator=(const raii::surface_khr&) noexcept -> raii::surface_khr& = delete;

    surface_khr(raii::surface_khr&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, instance_{other.instance_} {}

    auto operator=(raii::surface_khr&& other) noexcept -> raii::surface_khr& {
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

[[nodiscard]] inline auto make_surface_khr(hgn::surface_khr&& native, hgn::instance& inst) noexcept
    -> raii::surface_khr {
    return raii::surface_khr::make(std::move(native), inst);
}
}  // namespace hgn::raii