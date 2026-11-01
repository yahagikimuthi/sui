#pragma once

#include "hgn/detail/instance.hpp"

namespace hgn {
[[nodiscard]] inline auto get_instance_creator() noexcept -> detail::instance_creator {
    return detail::instance_creator{};
}
}  // namespace hgn