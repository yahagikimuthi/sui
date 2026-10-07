#pragma once

#include "hgn/types.hpp"

namespace hgn {
enum class result : u8 {
    success,
    incomplete,
    error_out_of_host_memory,
    error_initialization_failed
};

using result::error_out_of_host_memory;
using result::incomplete;
using result::success;
}  // namespace hgn