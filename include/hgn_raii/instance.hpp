#pragma once

#include <expected>
#include <optional>
#include <utility>

#include <hgn/enums.hpp>
#include <hgn/instance.hpp>

namespace hgn::raii {
class instance final {
  public:
    [[nodiscard]] static auto try_make(hgn::instance_create_info& info) noexcept
        -> std::expected<instance, result> {
        auto res = hgn::try_make_instance(info);
        if (not res) return std::unexpected{res.error()};

        return instance{*res};
    }

    instance(const instance&) noexcept                    = delete;
    auto operator=(const instance&) noexcept -> instance& = delete;

    instance(instance&& other) noexcept : native_{std::exchange(other.native_, std::nullopt)} {}
    auto operator=(instance&& other) noexcept -> instance& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        return *this;
    }

    ~instance() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::instance& { return *native_; }

    [[nodiscard]] operator hgn::instance&() noexcept { return get(); }

  private:
    explicit instance(hgn::instance& native) noexcept : native_{native} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_instance(get());
    }

    std::optional<hgn::instance&> native_;
};
}  // namespace hgn::raii