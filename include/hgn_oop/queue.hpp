#pragma once

#include <cstdint>
#include <expected>
#include <optional>

#include <hgn/enums.hpp>
#include <hgn/submit.hpp>
#include <hgn_raii/queue.hpp>

#include "hgn_oop/hardware.hpp"
#include "hgn_oop/sync.hpp"

namespace hgn::oop {
class queue final {
  public:
    [[nodiscard]] static auto try_make(oop::device& dev, std::uint32_t family_indices) noexcept
        -> std::optional<queue> {
        auto native = raii::try_get_queue(dev.get(), family_indices);
        if (not native) return std::nullopt;
        return oop::queue{*native};
    }

    [[nodiscard]] auto try_submit(const hgn::submit_info& info, oop::fence& fence_ref) noexcept
        -> std::expected<void, hgn::result> {
        return hgn::try_queue_submit(native_, info, fence_ref.get());
    }

    [[nodiscard]] auto get() noexcept -> raii::queue& { return native_; }

  private:
    explicit queue(const raii::queue& native) noexcept : native_{native} {}

    raii::queue native_;
};
}  // namespace hgn::oop