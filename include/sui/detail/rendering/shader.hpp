#pragma once

#include <expected>
#include <optional>
#include <span>
#include <utility>

#include <hgn/device.hpp>
#include <hgn/shader.hpp>
#include <hgn_raii/device.hpp>
#include <hgn_raii/shader.hpp>

#include "sui/detail/others.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class shader final {
  public:
    [[nodiscard]] static auto try_make(hgn::raii::device& dev) noexcept
        -> std::expected<shader, error> {
        const auto vert_source = read_file("../include/sui/detail/rendering/vert.spv");
        if (not vert_source) return make_error(logic_error, "Failed to read vertex source file.");
        const auto vert_info = hgn::shader_module_create_info{}.code(*vert_source);
        auto       vert      = hgn::raii::try_make_shader_module(vert_info, dev);
        if (not vert) return make_error(runtime_error, "Failed to make vertex shader module.");

        const auto frag_source = read_file("../include/sui/detail/rendering/frag.spv");
        if (not frag_source) return make_error(logic_error, "Failed to read fragment source file.");
        const auto frag_info = hgn::shader_module_create_info{}.code(*frag_source);
        auto       frag      = hgn::raii::try_make_shader_module(frag_info, dev);
        if (not frag) return make_error(runtime_error, "Failed to make fragment shader module.");

        return shader{std::move(*vert), std::move(*frag)};
    }

    shader(const shader&) noexcept                    = delete;
    auto operator=(const shader&) noexcept -> shader& = delete;
    shader(shader&&) noexcept                         = default;
    auto operator=(shader&&) noexcept -> shader&      = default;
    ~shader() noexcept                                = default;

    [[nodiscard]] auto vertex_shader() noexcept -> hgn::raii::shader_module& { return vert_; }

    [[nodiscard]] auto fragment_shader() noexcept -> hgn::raii::shader_module& { return frag_; }

  private:
    explicit shader(hgn::raii::shader_module&& vert, hgn::raii::shader_module&& frag) noexcept
        : vert_{std::move(vert)}, frag_{std::move(frag)} {}

    hgn::raii::shader_module vert_;
    hgn::raii::shader_module frag_;
};
}  // namespace sui::detail