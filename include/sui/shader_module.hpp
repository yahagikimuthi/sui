#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <span>
#include <utility>

#include <hgn/device.hpp>
#include <hgn/shader.hpp>

#include "sui/error.hpp"

namespace sui {
class shader_module final {
  public:
    [[nodiscard]] static auto try_make(hgn::device& dev, const std::span<const char> code) noexcept
        -> std::expected<shader_module, error> {
        const auto info = hgn::shader_module_create_info{}.code(code);
        const auto res  = hgn::try_make_shader_module(dev, info);
        if (not res) return make_error(runtime_error, "Failed to make shader module.");

        return shader_module{res->get(), dev};
    }

    shader_module(const shader_module&) noexcept = delete;

    auto operator=(const shader_module&) noexcept -> shader_module& = delete;

    shader_module(shader_module&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(shader_module&& other) noexcept -> shader_module& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;

        return *this;
    }

    ~shader_module() noexcept { destroy(); }

  private:
    explicit shader_module(hgn::shader_module& native, hgn::device& dev) noexcept
        : native_{native}, dev_{dev} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_shader_module(dev_.get(), *native_);
    }

    std::optional<hgn::shader_module&>  native_;
    std::reference_wrapper<hgn::device> dev_;
};
}  // namespace sui