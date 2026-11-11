#pragma once

#include <expected>

#include <hgn/image.hpp>

#include "sui/error.hpp"

namespace sui {
class texture final {
  public:
    static auto make() noexcept -> std::expected<texture, error> {}

  private:
    hgn::borrowed_image raw_;
};
}  // namespace sui