#pragma once

#include <expected>
#include <utility>

#include <kgm/window.hpp>

#include "sui/error.hpp"
#include "sui/hardware.hpp"
#include "sui/protocol.hpp"
#include "sui/render_ctx.hpp"

namespace sui {
class vk_context final {
  public:
    [[nodiscard]] static auto try_make(kgm::window& win) noexcept
        -> std::expected<vk_context, error> {
        auto hard_res = hardware::try_make(win);
        if (not hard_res)
            return make_error(runtime_error, "Failed to make hardware object.", hard_res.error());

        auto render_res = render_context::try_make(*hard_res);
        if (not render_res)
            return make_error(
                runtime_error, "Failed to make render context class.", render_res.error()
            );

        auto protocol_res = protocol::try_make(
            hard_res->native_device(), hard_res->get_physical_device().graphics_family()
        );
        if (not protocol_res)
            return make_error(
                runtime_error, "Failed to make protocol object.", protocol_res.error()
            );

        return vk_context{std::move(*hard_res), std::move(*render_res), std::move(*protocol_res)};
    }

  private:
    explicit vk_context(hardware&& hard, render_context&& render, protocol&& prot) noexcept
        : hard_{std::move(hard)}, render_{std::move(render)}, protocol_{std::move(prot)} {}

    hardware       hard_;
    render_context render_;
    protocol       protocol_;
};
}  // namespace sui