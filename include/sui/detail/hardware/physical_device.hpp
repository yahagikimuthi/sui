#pragma once

#include <cstdint>
#include <string>

#include <hgn_oop/hardware.hpp>
#include <hgn_raii/device.hpp>

namespace sui::detail {
class physical_device final {
  public:
    explicit physical_device(
        const hgn::oop::physical_device& native,
        const std::uint32_t              graphics_family,
        const std::uint32_t              present_family
    ) noexcept
        : native_{native}, graphics_family_{graphics_family}, present_family_{present_family} {}

    [[nodiscard]] auto name() noexcept -> std::string {
        return std::string{native_.get_properties().name()};
    }

    [[nodiscard]] auto graphics_family() const noexcept -> std::uint32_t {
        return graphics_family_;
    }

    [[nodiscard]] auto present_family() const noexcept -> std::uint32_t { return present_family_; }

    [[nodiscard]] auto get() noexcept -> hgn::oop::physical_device& { return native_; }

  private:
    hgn::oop::physical_device native_;
    std::uint32_t             graphics_family_;
    std::uint32_t             present_family_;
};
}  // namespace sui::detail