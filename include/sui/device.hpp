#pragma once

#include <hgn/device.hpp>

namespace sui {
class device final {
  public:
    explicit device(hgn::physical_device& physical_device) noexcept;

  private:
};
}  // namespace sui