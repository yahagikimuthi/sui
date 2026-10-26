#pragma once

#include <functional>
#include <utility>

#include <hgn/physical_device.hpp>

namespace hgn::raii {
class physical_device final {
  public:
    [[nodiscard]] static auto make(hgn::physical_device&& native) noexcept -> physical_device {
        return physical_device{std::move(native)};
    }

    physical_device(hgn::physical_device&& native) noexcept : native_{native} {}

    [[nodiscard]] auto get() noexcept -> hgn::physical_device& { return native_; }

    [[nodiscard]] operator hgn::physical_device&() noexcept { return get(); }

  private:
    std::reference_wrapper<hgn::physical_device> native_;
};

[[nodiscard]] inline auto make_physical(hgn::physical_device&& native) noexcept -> physical_device {
    return physical_device::make(std::move(native));
}
}  // namespace hgn::raii