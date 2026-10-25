#pragma once

#include <expected>
#include <span>
#include <utility>

#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/physical_device.hpp>
#include <hgn/pipeline.hpp>

#include "sui/detail/rendering/buffer.hpp"
#include "sui/detail/rendering/pipeline.hpp"
#include "sui/detail/rendering/shader.hpp"
#include "sui/error.hpp"

namespace sui {
class renderer final {
  public:
    [[nodiscard]] static auto try_make(
        hgn::device& dev, hgn::physical_device& physical, const hgn::format fmt
    ) noexcept -> std::expected<renderer, error> {
        auto shader_res = shader::try_make(dev);
        if (not shader_res)
            return make_error(runtime_error, "Failed to make shader.", shader_res.error());

        auto pipe_res = graphics_pipeline::try_make(
            dev, shader_res->vertex_shader(), shader_res->fragment_shader(), fmt
        );
        if (not pipe_res)
            return make_error(runtime_error, "Failed to make pipeline.", pipe_res.error());

        auto buffer_res = buffer::try_make(dev, physical);
        if (not buffer_res)
            return make_error(runtime_error, "Failed to make buffer.", buffer_res.error());

        return renderer{std::move(*shader_res), std::move(*pipe_res), std::move(*buffer_res)};
    }

    renderer(const renderer&) noexcept                    = delete;
    auto operator=(const renderer&) noexcept -> renderer& = delete;
    renderer(renderer&&) noexcept                         = default;
    auto operator=(renderer&&) noexcept -> renderer&      = default;
    ~renderer() noexcept                                  = default;

    void update(const std::span<const vertex> vertices) noexcept { buf_.update(vertices); }

    [[nodiscard]] auto get_buffer() noexcept -> hgn::buffer& { return buf_.get_buffer(); }

    [[nodiscard]] auto get_pipeline() noexcept -> hgn::pipeline& { return pipe_.get_pipeline(); }

  private:
    explicit renderer(shader&& shad, graphics_pipeline&& pipe, buffer&& buf) noexcept
        : shader_{std::move(shad)}, pipe_{std::move(pipe)}, buf_{std::move(buf)} {}

    shader            shader_;
    graphics_pipeline pipe_;
    buffer            buf_;
};
}  // namespace sui