#pragma once

#include <utility>

#include "hgn/detail/queue.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class queue final {
    friend class device;

  public:
    [[nodiscard]] auto get_present_executor() noexcept -> present_executor {
        return present_executor{get()};
    }

    [[nodiscard]] auto get_submitter() noexcept -> submitter { return submitter{get()}; }

    [[nodiscard]] auto get() noexcept -> raw::queue& { return raw_; }

  private:
    explicit queue(raw::queue& raw) noexcept : raw_{std::move(raw)} {}

    raii::queue raw_;
};
}  // namespace hgn