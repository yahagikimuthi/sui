#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>

namespace sui {
class renderer final {
  public:
    [[nodiscard]] static auto create() noexcept;

  private:
};
}  // namespace sui