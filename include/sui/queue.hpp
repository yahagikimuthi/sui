#pragma once

#include <cstdint>
#include <expected>
#include <functional>

#include <hgn/device.hpp>

#include "sui/error.hpp"

namespace sui {
class queue final {
  public:
    [[nodiscard]] static auto try_make(
        hgn::device& dev, const std::uint32_t family_indices
    ) noexcept -> std::expected<queue, error> {
        auto res = hgn::try_get_device_queue(dev, family_indices);
        if (not res) return make_error(runtime_error, "Failed to make queue.");

        return queue{*res};
    }

    [[nodiscard]] auto native() noexcept -> hgn::queue& { return native_.get(); }

  private:
    explicit queue(hgn::queue& native) : native_{native} {}

    std::reference_wrapper<hgn::queue> native_;
};
}  // namespace sui