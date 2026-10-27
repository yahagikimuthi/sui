#pragma once

#include <expected>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/sync.hpp>
#include <hgn_raii/sync.hpp>

#include "hgn_oop/hardware.hpp"

namespace hgn::oop {
class semaphore final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::semaphore_create_info& info, oop::device& dev
    ) noexcept -> std::expected<oop::semaphore, hgn::result> {
        auto native = raii::try_make_semaphore(info, dev.get());
        if (not native) return std::unexpected{native.error()};

        return oop::semaphore{std::move(*native)};
    }

    [[nodiscard]] auto get() noexcept -> raii::semaphore& { return native_; }

  private:
    explicit semaphore(raii::semaphore&& native) noexcept : native_{std::move(native)} {}

    raii::semaphore native_;
};

class fence final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::fence_create_info& info, oop::device& dev
    ) noexcept -> std::expected<oop::fence, hgn::result> {
        auto native = raii::try_make_fence(info, dev.get());
        if (not native) return std::unexpected{native.error()};

        return oop::fence{std::move(*native)};
    }

    [[nodiscard]] auto try_wait(hgn::device& dev) noexcept -> std::expected<void, hgn::result> {
        return hgn::try_wait_for_fence(dev, native_);
    }

    [[nodiscard]] auto get() noexcept -> raii::fence& { return native_; }

  private:
    explicit fence(raii::fence&& native) noexcept : native_{std::move(native)} {}

    raii::fence native_;
};
}  // namespace hgn::oop