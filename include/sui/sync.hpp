#pragma once

#include <cassert>
#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/device.hpp>
#include <hgn/sync.hpp>

#include "sui/error.hpp"

namespace sui {
class semaphore final {
  public:
    [[nodiscard]] static auto try_make(hgn::device& dev) noexcept
        -> std::expected<semaphore, error> {
        const auto res = hgn::try_make_semaphore(dev, hgn::semaphore_create_info{});
        if (not res) return make_error(runtime_error, "Failed to make semaphore");
        return semaphore{res->get(), dev};
    }

    semaphore(const semaphore&) noexcept = delete;

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

    ~semaphore() noexcept = default;

    [[nodiscard]] auto native() noexcept -> hgn::semaphore& { return native_.value(); }

  private:
    explicit semaphore(hgn::semaphore& native, hgn::device& dev) noexcept
        : native_{native}, dev_{dev} {}

    void destroy() noexcept {
        if (not native_) return;
        hgn::destroy_semaphore(dev_.get(), *native_);
    }

    std::optional<hgn::semaphore&>      native_;
    std::reference_wrapper<hgn::device> dev_;
};

class fence final {
  public:
    [[nodiscard]] static auto try_make(hgn::device& dev) noexcept -> std::expected<fence, error> {
        const auto res = hgn::try_make_fence(dev, hgn::fence_create_info{});
        if (not res) return make_error(runtime_error, "Failed to make fence");

        return fence{res->get(), dev};
    }

    fence(const fence&) noexcept = delete;

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

    ~fence() noexcept = default;

    [[nodiscard]] auto native() noexcept -> hgn::fence& { return native_.value(); }

  private:
    explicit fence(hgn::fence& native, hgn::device& dev) noexcept : native_{native}, dev_{dev} {}

    void destroy() noexcept {
        if (not native_) return;
        hgn::destroy_fence(dev_.get(), *native_);
    }

    std::optional<hgn::fence&>          native_;
    std::reference_wrapper<hgn::device> dev_;
};

class sync final {
  public:
    [[nodiscard]] static auto try_make(hgn::device& dev) noexcept -> std::expected<sync, error> {
        auto sem1 = semaphore::try_make(dev);
        auto sem2 = semaphore::try_make(dev);
        auto fen  = fence::try_make(dev);

        if (not sem1) return make_error(runtime_error, "Failed to make image available semaphore.");
        if (not sem2) return make_error(runtime_error, "Failed to make render finished semaphore.");
        if (not fen) return make_error(runtime_error, "Failed to make fence.");

        return sync{std::move(*sem1), std::move(*sem2), std::move(*fen)};
    }

    [[nodiscard]] auto image_available_semaphore() noexcept -> hgn::semaphore& {
        return image_available_.native();
    }

    [[nodiscard]] auto render_finished_semaphore() noexcept -> hgn::semaphore& {
        return render_finished_.native();
    }

    [[nodiscard]] auto in_flight_fence() noexcept -> hgn::fence& { return fence_.native(); }

  private:
    explicit sync(semaphore&& image, semaphore&& render, fence&& fen) noexcept
        : image_available_{std::move(image)},
          render_finished_{std::move(render)},
          fence_{std::move(fen)} {}

    semaphore image_available_;
    semaphore render_finished_;
    fence     fence_;
};
}  // namespace sui