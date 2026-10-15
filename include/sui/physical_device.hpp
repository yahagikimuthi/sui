#pragma once

#include <optional>

#include <hgn/physical_device.hpp>
#include <hgn/surface.hpp>

namespace sui {
class physical_device final {
  public:
    explicit physical_device(
        hgn::physical_device& native,
        const std::uint32_t   graphics_family,
        const std::uint32_t   present_family
    ) noexcept
        : native_{native}, graphics_family_{graphics_family}, present_family_{present_family} {}

    [[nodiscard]] auto native() noexcept -> hgn::physical_device& { return native_.value(); }
    [[nodiscard]] auto graphics_family() const noexcept -> std::uint32_t {
        return graphics_family_.value();
    }
    [[nodiscard]] auto present_family() const noexcept -> std::uint32_t {
        return present_family_.value();
    }

  private:
    std::optional<hgn::physical_device&> native_;
    std::optional<std::uint32_t>         graphics_family_;
    std::optional<std::uint32_t>         present_family_;
};
}  // namespace sui