#pragma once

#include <cstdint>
#include <expected>

#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/submit.hpp>
#include <hgn_raii/queue.hpp>

#include "sui/detail/hardware/device.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class queue final {
  public:
    [[nodiscard]] static auto try_make(device& dev, const std::uint32_t family_indices) noexcept
        -> std::expected<queue, error> {
        auto res = dev.try_get_queue(family_indices);
        if (not res) return make_error(runtime_error, "Failed to make queue.");

        return queue{*res};
    }

    [[nodiscard]] auto try_present(const hgn::present_info_khr& info) noexcept
        -> std::expected<void, hgn::result> {
        const auto res = hgn::try_queue_present_khr(native_, info);
        return res;
    }

    [[nodiscard]] auto get() noexcept -> hgn::queue& { return native_; }

    [[nodiscard]] operator hgn::queue&() noexcept { return get(); }

  private:
    explicit queue(hgn::raii::queue& native) noexcept : native_{native} {}

    hgn::raii::queue native_;
};
}  // namespace sui::detail