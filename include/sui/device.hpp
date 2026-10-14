#pragma once

#include <optional>
#include <utility>

#include <hgn/device.hpp>

namespace sui {
class device final {
  public:
    explicit device(hgn::device& native) noexcept : native_{native} {}

    device(const device&) noexcept = delete;

    auto operator=(const device&) noexcept -> device& = delete;

    device(device&& other) noexcept : native_{std::exchange(other.native_, std::nullopt)} {}

    auto operator=(device&& other) noexcept -> device& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);

        return *this;
    }

    ~device() noexcept = default;

    [[nodiscard]] auto native() noexcept -> hgn::device& { return native_.value(); }

  private:
    void destroy() noexcept {
        if (native_) hgn::destroy_device(*native_);
    }

    std::optional<hgn::device&> native_;
};
}  // namespace sui