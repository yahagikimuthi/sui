#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/declaration.hpp>
#include <hgn/enums.hpp>
#include <hgn/sync.hpp>

namespace hgn::raii {
class semaphore final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::semaphore_create_info& info, hgn::device& dev
    ) noexcept -> std::expected<raii::semaphore, hgn::result> {
        auto res = hgn::try_make_semaphore(dev, info);
        if (not res) return std::unexpected{res.error()};

        return raii::semaphore{*res, dev};
    }

    semaphore(const raii::semaphore&) noexcept                          = delete;
    auto operator=(const raii::semaphore&) noexcept -> raii::semaphore& = delete;

    semaphore(raii::semaphore&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(raii::semaphore&& other) noexcept -> raii::semaphore& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;
        return *this;
    }

    ~semaphore() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::semaphore& { return *native_; }

    [[nodiscard]] operator hgn::semaphore&() noexcept { return get(); }

  private:
    explicit semaphore(hgn::semaphore& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_semaphore(dev_, get());
    }

    std::optional<hgn::semaphore&>      native_;
    std::reference_wrapper<hgn::device> dev_;
};

[[nodiscard]] inline auto try_make_semaphore(
    const hgn::semaphore_create_info& info, hgn::device& dev
) noexcept -> std::expected<raii::semaphore, hgn::result> {
    return raii::semaphore::try_make(info, dev);
}

class fence final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::fence_create_info& info, hgn::device& dev
    ) noexcept -> std::expected<raii::fence, hgn::result> {
        auto res = hgn::try_make_fence(dev, info);
        if (not res) return std::unexpected{res.error()};

        return raii::fence{*res, dev};
    }

    fence(const raii::fence&) noexcept                          = delete;
    auto operator=(const raii::fence&) noexcept -> raii::fence& = delete;

    fence(raii::fence&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(raii::fence&& other) noexcept -> raii::fence& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;
        return *this;
    }

    ~fence() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::fence& { return *native_; }

    [[nodiscard]] operator hgn::fence&() noexcept { return get(); }

  private:
    explicit fence(hgn::fence& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_fence(dev_, get());
    }

    std::optional<hgn::fence&>          native_;
    std::reference_wrapper<hgn::device> dev_;
};

[[nodiscard]] inline auto try_make_fence(
    const hgn::fence_create_info& info, hgn::device& dev
) noexcept -> std::expected<raii::fence, hgn::result> {
    return raii::fence::try_make(info, dev);
}
}  // namespace hgn::raii