#pragma once

#include <cassert>
#include <exception>
#include <expected>
#include <functional>
#include <iostream>
#include <optional>
#include <utility>

#include <hgn/device.hpp>
#include <hgn/sync.hpp>

#include "sui/debugger.hpp"
#include "sui/detail/debug_impl.hpp"
#include "sui/detail/hardware/device.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class semaphore final {
  public:
    [[nodiscard]] static auto try_make(device& dev) noexcept -> std::expected<semaphore, error> {
        const auto res = dev.try_make_semaphore(hgn::semaphore_create_info{});
        if (not res) return make_error(runtime_error, "Failed to make semaphore");
        return semaphore{*res, dev};
    }

    semaphore(const semaphore&) noexcept                    = delete;
    auto operator=(const semaphore&) noexcept -> semaphore& = delete;

    semaphore(semaphore&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(semaphore&& other) noexcept -> semaphore& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;

        return *this;
    }

    ~semaphore() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::semaphore& { return native_.value(); }

  private:
    explicit semaphore(hgn::semaphore& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (not native_) return;
        hgn::destroy_semaphore(dev_.get(), *native_);
    }

    std::optional<hgn::semaphore&>      native_;
    std::reference_wrapper<hgn::device> dev_;
};

class fence final {
  public:
    [[nodiscard]] static auto try_make(device& dev) noexcept -> std::expected<fence, error> {
        const auto res = dev.try_make_fence(hgn::fence_create_info{});
        if (not res) return make_error(runtime_error, "Failed to make fence");

        return fence{*res, dev};
    }

    fence(const fence&) noexcept                    = delete;
    auto operator=(const fence&) noexcept -> fence& = delete;

    fence(fence&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(fence&& other) noexcept -> fence& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;

        return *this;
    }

    ~fence() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::fence& { return native_.value(); }

  private:
    explicit fence(hgn::fence& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (not native_) return;
        if (not hgn::try_wait_for_fence(dev_.get(), *native_)) {
            std::cerr << "Failed to wait fence\n";
            std::terminate();
        }
        hgn::destroy_fence(dev_.get(), *native_);
    }

    std::optional<hgn::fence&>          native_;
    std::reference_wrapper<hgn::device> dev_;
};

class sync final {
  public:
    [[nodiscard]] static auto try_make(device& dev) noexcept -> std::expected<sync, error> {
        auto sem1 = semaphore::try_make(dev);
        auto sem2 = semaphore::try_make(dev);
        auto fen  = fence::try_make(dev);

        if (not sem1)
            return make_error(
                runtime_error, "Failed to make image available semaphore.", sem1.error()
            );
        if (not sem2)
            return make_error(
                runtime_error, "Failed to make render finished semaphore.", sem2.error()
            );
        if (not fen) return make_error(runtime_error, "Failed to make fence.", fen.error());

        return sync{std::move(*sem1), std::move(*sem2), std::move(*fen)};
    }

    sync(const sync&) noexcept                    = delete;
    auto operator=(const sync&) noexcept -> sync& = delete;
    sync(sync&&) noexcept                         = default;
    auto operator=(sync&&) noexcept -> sync&      = default;
    ~sync() noexcept                              = default;

    [[nodiscard]] auto try_wait_fence(hgn::device& dev) noexcept -> bool {
        auto wait_res = hgn::try_wait_for_fence(dev, in_flight_fence());
        if (not wait_res) return fail("Failed to wait for fence.");

        auto reset_res = hgn::try_reset_fence(dev, in_flight_fence());
        if (not reset_res) return fail("Failed to reset fence.");

        return true;
    }

    [[nodiscard]] auto image_available_semaphore() noexcept -> hgn::semaphore& {
        return image_available_.get();
    }

    [[nodiscard]] auto render_finished_semaphore() noexcept -> hgn::semaphore& {
        return render_finished_.get();
    }

    [[nodiscard]] auto in_flight_fence() noexcept -> hgn::fence& { return fence_.get(); }

  private:
    explicit sync(semaphore&& image, semaphore&& render, fence&& fen) noexcept
        : image_available_{std::move(image)},
          render_finished_{std::move(render)},
          fence_{std::move(fen)} {}

    semaphore image_available_;
    semaphore render_finished_;
    fence     fence_;
};
}  // namespace sui::detail