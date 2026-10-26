#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/shader.hpp>

namespace hgn::raii {
class shader_module final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::shader_module_create_info& info, hgn::device& dev
    ) noexcept -> std::expected<shader_module, hgn::result> {
        auto res = hgn::try_make_shader_module(dev, info);
        if (not res) return std::unexpected{res.error()};

        return shader_module{*res, dev};
    }

    shader_module(const shader_module&) noexcept                    = delete;
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

    [[nodiscard]] auto get() noexcept -> hgn::shader_module& { return *native_; }

    [[nodiscard]] operator hgn::shader_module&() noexcept { return get(); }

  private:
    explicit shader_module(hgn::shader_module& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_shader_module(dev_, get());
    }

    std::optional<hgn::shader_module&>  native_;
    std::reference_wrapper<hgn::device> dev_;
};
}  // namespace hgn::raii