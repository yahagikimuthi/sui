#pragma once

#include <functional>

#include <hgn/physical_device.hpp>

namespace hgn::raii {
class physical_device final {
  public:
    physical_device(hgn::physical_device&& native) noexcept : native_{native} {}

    [[nodiscard]] auto get() noexcept -> hgn::physical_device& { return native_; }

    [[nodiscard]] operator hgn::physical_device&() noexcept { return get(); }

  private:
    std::reference_wrapper<hgn::physical_device> native_;
};
}  // namespace hgn::raii