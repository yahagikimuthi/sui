#pragma once

#include <hgn/device.hpp>
#include <hgn/queue.hpp>

namespace sui {
class queue_container final {
  public:
    queue_container(hgn::device& dev, hgn::queue_family family) noexcept
        : family_{family}, queue_{dev.get_queue(family)} {}

    [[nodiscard]] auto family() noexcept -> hgn::queue_family& { return family_; }
    [[nodiscard]] auto queue() noexcept -> hgn::queue& { return queue_; }

  private:
    hgn::queue_family family_;
    hgn::queue        queue_;
};
}  // namespace sui