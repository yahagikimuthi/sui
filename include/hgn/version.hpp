#pragma once

#include "hgn/types.hpp"

namespace hgn {
struct version_t {
    u32 major{};
    u32 minor{};
    u32 patch{};
};

struct api_version_t {
    u32 variant{};
    u32 major{};
    u32 minor{};
    u32 patch{};
};

inline constexpr auto api_version_1_3 =
    api_version_t{.variant = 0, .major = 0, .minor = 3, .patch = 0};
}  // namespace hgn