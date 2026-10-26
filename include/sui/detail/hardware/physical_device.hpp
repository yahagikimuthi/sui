#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <hgn/device.hpp>
#include <hgn/physical_device.hpp>
#include <hgn_raii/device.hpp>

#include "sui/detail/others.hpp"

namespace sui::detail {
class physical_device final {
  public:
    explicit physical_device(
        const hgn::raii::physical_device& native,
        const std::uint32_t               graphics_family,
        const std::uint32_t               present_family
    ) noexcept
        : native_{native}, graphics_family_{graphics_family}, present_family_{present_family} {}

    [[nodiscard]] auto name() noexcept -> std::string {
        const auto prop = hgn::get_physical_device_properties(get());
        return std::string{prop.name()};
    }

    [[nodiscard]] auto get_physical_device_memory_properties() noexcept
        -> hgn::physical_device_memory_properties {
        return hgn::get_physical_device_memory_properties(get());
    }

    [[nodiscard]] auto try_make_device(const hgn::device_create_info& info) noexcept
        -> std::optional<hgn::raii::device> {
        auto res = hgn::raii::try_make_device(info, get());
        return to_optional(std::move(res));
    }

    [[nodiscard]] auto graphics_family() const noexcept -> std::uint32_t {
        return graphics_family_;
    }

    [[nodiscard]] auto present_family() const noexcept -> std::uint32_t { return present_family_; }

    [[nodiscard]] auto get() noexcept -> hgn::raii::physical_device& { return native_; }

    [[nodiscard]] operator hgn::raii::physical_device&() noexcept { return get(); }

  private:
    hgn::raii::physical_device native_;
    std::uint32_t              graphics_family_;
    std::uint32_t              present_family_;
};
}  // namespace sui::detail