#pragma once

#include <functional>
#include <utility>

#include <hgn/device.hpp>

namespace hgn::raii {
class queue final {
  public:
    [[nodiscard]] static auto make(hgn::queue&& native) noexcept -> queue {
        return queue{std::move(native)};
    }

    explicit queue(hgn::queue&& native) noexcept : native_{native} {}

    [[nodiscard]] auto get() noexcept -> hgn::queue& { return native_; }

    [[nodiscard]] operator hgn::queue&() noexcept { return get(); }

  private:
    std::reference_wrapper<hgn::queue> native_;
};

[[nodiscard]] inline auto make_queue(hgn::queue&& native) noexcept -> queue {
    return queue::make(std::move(native));
}
}  // namespace hgn::raii