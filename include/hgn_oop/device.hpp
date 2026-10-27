#pragma once

#include <expected>
#include <hgn_raii/device.hpp>

namespace hgn::oop {
class device final {
  public:
    [[nodiscard]] auto try_make() noexcept -> std::expected<device, hgn::result>;

  private:
    hgn::raii::device native_;
};
}  // namespace hgn::oop