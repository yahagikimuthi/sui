#pragma once

#include <hgn/image.hpp>

namespace wk::detail {
struct image final {
    hgn::image      img;
    hgn::image_view view;
};
}  // namespace wk::detail