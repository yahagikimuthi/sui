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
    ) noexcept -> std::expected<semaphore, hgn::result> {
        auto res = hgn::try_make_semaphore(dev, info);
        if (not res) return std::unexpected{res.error()};

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
) noexcept -> std::expected<semaphore, hgn::result> {
    return semaphore::try_make(info, dev);
}

class fence final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::fence_create_info& info, hgn::device& dev
    ) noexcept -> std::expected<fence, hgn::result> {
        auto res = hgn::try_make_fence(dev, info);
        if (not res) return std::unexpected{res.error()};

        return fence{*res, dev};
    }

    fence(const fence&) noexcept                    = delete;
    auto operator=(const fence&) noexcept -> fence& = delete;

    fence(fence&& other) noexcept : native_{other.native_}, dev_{other.dev_} {}

    auto operator=(fence&& other) noexcept -> fence& {
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
) noexcept -> std::expected<fence, hgn::result> {
    return fence::try_make(info, dev);
}
}  // namespace hgn::raii