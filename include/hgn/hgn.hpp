#pragma once

#include "hgn/detail/hgn.hpp"

namespace hgn {
[[nodiscard]] inline auto get_instance_creator() noexcept -> instance_creator {
    return instance_creator{};
}

static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);
}  // namespace hgn