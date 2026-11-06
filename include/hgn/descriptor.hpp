#pragma once

#include <utility>

#include "hgn/raii/rendering.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class descriptor_set_layout final {
    friend class device;

  public:
    [[nodiscard]] auto get() noexcept -> raw::descriptor_set_layout& { return raw_; }

  private:
    explicit descriptor_set_layout(raw::descriptor_set_layout& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::descriptor_set_layout raw_;
};

class descriptor_pool final {
  public:
    friend class device;

    [[nodiscard]] auto get() noexcept -> raw::descriptor_pool& { return raw_; }

  private:
    explicit descriptor_pool(raw::descriptor_pool& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::descriptor_pool raw_;
};

class descriptor_set final {
    friend class device;

  public:
    [[nodiscard]] auto get() noexcept -> raw::descriptor_set& { return raw_; }

  private:
    explicit descriptor_set(raw::descriptor_set& raw) noexcept : raw_{raw} {}

    raii::descriptor_set raw_;
};
}  // namespace hgn