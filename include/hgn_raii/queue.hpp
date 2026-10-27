#pragma once

#include <cstdint>
#include <functional>
#include <optional>

#include <hgn/declaration.hpp>
#include <hgn/device.hpp>

namespace hgn::raii {
class queue final {
  public:
    [[nodiscard]] static auto try_get(hgn::device& dev, const std::uint32_t family_indices) noexcept
        -> std::optional<queue> {
        auto res = hgn::try_get_device_queue(dev, family_indices);
        if (not res) return std::nullopt;
        return queue{*res};
    }

    [[nodiscard]] auto get() noexcept -> hgn::queue& { return native_; }

    [[nodiscard]] operator hgn::queue&() noexcept { return get(); }

  private:
    explicit queue(hgn::queue& native) noexcept : native_{std::ref(native)} {}

    std::reference_wrapper<hgn::queue> native_;
};

[[nodiscard]] inline auto try_get_queue(
    hgn::device& dev, const std::uint32_t family_indices
) noexcept -> std::optional<queue> {
    return queue::try_get(dev, family_indices);
}
}  // namespace hgn::raii