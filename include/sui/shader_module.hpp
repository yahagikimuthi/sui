#pragma once

#include <expected>
#include <optional>
#include <span>

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

        return shader_module{res->get()};
    }

  private:
    explicit shader_module(hgn::shader_module& native) : native_{native} {}
    std::optional<hgn::shader_module&> native_;
};
}  // namespace sui