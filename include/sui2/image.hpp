#pragma once

#include <expected>

#include "sui2/error.hpp"

namespace sui {
class image final {
  public:
    [[nodiscard]] static auto make() noexcept -> std::expected<image, error>;

  private:
    explicit image() noexcept;
};
}  // namespace sui