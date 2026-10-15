#pragma once

#include <cassert>
#include <expected>
#include <optional>
#include <utility>

#include <hgn/sync.hpp>

#include "hgn/device.hpp"
#include "sui/device.hpp"
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
        : native_{std::exchange(other.native_, std::nullopt)},
          dev_{std::exchange(other.dev_, std::nullopt)} {}

    auto operator=(semaphore&& other) noexcept -> semaphore& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = std::exchange(other.dev_, std::nullopt);

        return *this;
    }

    ~semaphore() noexcept = default;

    [[nodiscard]] auto native() noexcept -> hgn::semaphore& { return native_.value(); }

  private:
    explicit semaphore(hgn::semaphore& native, hgn::device& dev) : native_{native}, dev_{dev} {}

    void destroy() noexcept {
        if (not native_) return;
        assert(dev_);
        hgn::destroy_semaphore(*dev_, *native_);
    }

    std::optional<hgn::semaphore&> native_;
    std::optional<hgn::device&>    dev_;
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
        : native_{std::exchange(other.native_, std::nullopt)},
          dev_{std::exchange(other.dev_, std::nullopt)} {}

    auto operator=(fence&& other) noexcept -> fence& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = std::exchange(other.dev_, std::nullopt);

        return *this;
    }

    ~fence() noexcept = default;

  private:
    explicit fence(hgn::fence& native, hgn::device& dev) : native_{native}, dev_{dev} {}

    void destroy() noexcept {
        if (not native_) return;
        assert(dev_);
        hgn::destroy_fence(*dev_, *native_);
    }

    std::optional<hgn::fence&>  native_;
    std::optional<hgn::device&> dev_;
};

class sync final {
  public:
  private:
    std::optional<hgn::semaphore&> image_available_;
    std::optional<hgn::semaphore&> render_finished_;
    std::optional<hgn::fence&>     fence_;
};
}  // namespace sui