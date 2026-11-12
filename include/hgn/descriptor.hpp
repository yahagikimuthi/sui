#pragma once

#include <functional>
#include <utility>

#include "hgn/buffer.hpp"
#include "hgn/detail/descriptor.hpp"
#include "hgn/raii/rendering.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class descriptor_set_layout final {
  public:
    explicit descriptor_set_layout(raw::descriptor_set_layout& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    [[nodiscard]] auto get() noexcept -> raw::descriptor_set_layout& { return raw_; }

  private:
    raii::descriptor_set_layout raw_;
};

class descriptor_set final {
  public:
    explicit descriptor_set(raw::descriptor_set& raw, raw::device& dev) noexcept
        : raw_{raw}, dev_{std::ref(dev)} {}

    [[nodiscard]] auto get_writer() noexcept -> descriptor_set_writer {
        return descriptor_set_writer{get(), dev_.get()};
    }

    [[nodiscard]] auto get() noexcept -> raw::descriptor_set& { return raw_; }

  private:
    raii::descriptor_set                raw_;
    std::reference_wrapper<raw::device> dev_;
};
}  // namespace hgn