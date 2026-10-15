#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/command.hpp>

#include "sui/error.hpp"

namespace sui {
class command_pool final {
    template <typename T>
    using ref_w = std::reference_wrapper<T>;

  public:
    [[nodiscard]] static auto try_make(
        hgn::device& dev, const std::uint32_t queue_family_indices
    ) noexcept -> std::expected<command_pool, error> {
        auto info = hgn::command_pool_create_info{};
        info.queue_family_index(queue_family_indices);

        auto res = hgn::try_make_command_pool(dev, info);
        if (not res) return make_error(runtime_error, "Failed to make command pool.");

        return command_pool{*res, dev};
    }

    command_pool(const command_pool&) noexcept = delete;

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

    ~command_pool() noexcept = default;

    [[nodiscard]] auto native() noexcept -> hgn::command_pool& { return native_.value(); }

  private:
    explicit command_pool(hgn::command_pool& native, hgn::device& dev) noexcept
        : native_{native}, dev_{dev} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_command_pool(dev_.get(), *native_);
    }

    std::optional<hgn::command_pool&> native_;
    ref_w<hgn::device>                dev_;
};
}  // namespace sui