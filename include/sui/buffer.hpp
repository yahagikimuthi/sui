#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/buffer.hpp>

#include "sui/error.hpp"

namespace sui {
class buffer final {
  public:
    [[nodiscard]] auto try_make(hgn::device& dev) noexcept -> std::expected<buffer, error>;

    buffer(const buffer&) noexcept = delete;

    auto operator=(const buffer&) noexcept -> buffer& = delete;

    buffer(buffer&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(buffer&& other) noexcept -> buffer& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;

        return *this;
    }

    ~buffer() noexcept { destroy(); }

  private:
    explicit buffer(hgn::buffer& native, hgn::device& dev) noexcept : native_{native}, dev_{dev} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_buffer(dev_.get(), *native_);
    }

    std::optional<hgn::buffer&>         native_;
    std::reference_wrapper<hgn::device> dev_;
};
}  // namespace sui