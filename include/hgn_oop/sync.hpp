#pragma once

#include <expected>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/sync.hpp>
#include <hgn_raii/sync.hpp>

namespace hgn::oop {
class semaphore final {
  public:
    [[nodiscard]] auto get() noexcept -> raii::semaphore& { return native_; }

  private:
    explicit semaphore(raii::semaphore&& native) noexcept : native_{std::move(native)} {}

    friend class device;

    raii::semaphore native_;
};

class fence final {
    friend class device;

  public:
    [[nodiscard]] auto try_wait(hgn::device& dev) noexcept -> std::expected<void, hgn::result> {
        return hgn::try_wait_for_fence(dev, native_);
    }

    [[nodiscard]] auto get() noexcept -> raii::fence& { return native_; }

  private:
    explicit fence(raii::fence&& native) noexcept : native_{std::move(native)} {}

    raii::fence native_;
};
}  // namespace hgn::oop