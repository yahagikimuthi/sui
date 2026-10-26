#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/command.hpp>
#include <hgn/enums.hpp>

namespace hgn::raii {
class command_pool final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::command_pool_create_info& info, hgn::device& dev
    ) noexcept -> std::expected<command_pool, hgn::result> {
        auto res = hgn::try_make_command_pool(dev, info);
        if (not res) return std::unexpected{res.error()};
        return command_pool{*res, dev};
    }

    command_pool(const command_pool&) noexcept                    = delete;
    auto operator=(const command_pool&) noexcept -> command_pool& = delete;

    command_pool(command_pool&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(command_pool&& other) noexcept -> command_pool& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;
        return *this;
    }

    ~command_pool() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::command_pool& { return *native_; }

    [[nodiscard]] operator hgn::command_pool&() noexcept { return get(); }

  private:
    explicit command_pool(hgn::command_pool& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_command_pool(dev_, get());
    }

    std::optional<hgn::command_pool&>   native_;
    std::reference_wrapper<hgn::device> dev_;
};

[[nodiscard]] inline auto try_make_command_pool(
    const hgn::command_pool_create_info& info, hgn::device& dev
) noexcept -> std::expected<command_pool, hgn::result> {
    return command_pool::try_make(info, dev);
}
}  // namespace hgn::raii