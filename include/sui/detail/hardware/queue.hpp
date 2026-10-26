#pragma once

#include <cstdint>
#include <expected>
#include <functional>

#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/submit.hpp>
#include <hgn/sync.hpp>

#include "sui/detail/hardware/device.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class queue final {
  public:
    [[nodiscard]] static auto try_make(device& dev, const std::uint32_t family_indices) noexcept
        -> std::expected<queue, error> {
        auto res = dev.try_get_device_queue(family_indices);
        if (not res) return make_error(runtime_error, "Failed to make queue.");

        return queue{*res};
    }

    [[nodiscard]] auto try_submit(const hgn::submit_info& info, hgn::fence& fen) noexcept -> bool {
        const auto res = hgn::try_queue_submit(native_.get(), info, fen);
        return res.has_value();
    }

    [[nodiscard]] auto try_present(const hgn::present_info_khr& info) noexcept
        -> std::expected<void, hgn::result> {
        const auto res = hgn::try_queue_present_khr(native_.get(), info);
        return res;
    }

  private:
    explicit queue(hgn::queue& native) noexcept : native_{native} {}

    std::reference_wrapper<hgn::queue> native_;
};
}  // namespace sui::detail