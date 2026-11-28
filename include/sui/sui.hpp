#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string_view>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/buffer.hpp>
#include <hgn/descriptor.hpp>
#include <hgn/descriptor_pool.hpp>
#include <hgn/enums.hpp>
#include <hgn/rendering.hpp>
#include <kgm/kgm.hpp>
#include <kgm/window.hpp>

#include "sui/context.hpp"
#include "sui/info.hpp"

namespace sui {
class engine final {
  public:
    [[nodiscard]] static auto create(
        const std::uint32_t width, const std::uint32_t height, const std::string_view title
    ) noexcept -> std::expected<engine, akr::error> {
        auto win = kgm::window_creator{}.extent(width, height).title(title).create();
        if (not win) return win.error().rethrow_runtime("Failed to create window.");

        auto rs = rendering_system::create(*win);
        if (not rs) return rs.error().rethrow_runtime("Failed to create rendering system.");

        return engine{std::move(*win), std::move(*rs)};
    }

    [[nodiscard]] auto create_buffer(
        const std::uint64_t size, const hgn::buffer_usage_flag_bits usage
    ) noexcept -> std::expected<hgn::buffer, hgn::result> {
        return rs_.hard().allocator().get_buffer_creator().size(size).usage(usage).create();
    }

    [[nodiscard]] auto create_pipeline_layout(pipeline_layout_create_info& info
    ) noexcept -> std::expected<hgn::pipeline_layout, hgn::result> {
        auto creator = rs_.hard().device().get_pipeline_layout_creator();
        for (auto& layout : info.layouts) creator.add_layout(layout);
        for (const auto& push_constant : info.push_constants)
            creator.get_push_constant_adder()
                .offset(push_constant.offset)
                .size(push_constant.size)
                .stage_flags(push_constant.stage_flags)
                .push();

        return creator.create();
    }

    [[nodiscard]] auto create_graphics_pipeline(graphics_pipeline_create_info& info
    ) noexcept -> std::expected<hgn::graphics_pipeline, hgn::result> {
        auto creator = rs_.hard().device().get_graphics_pipeline_creator();
        creator.vertex_binding(info.vertex_struct_size);
        for (const auto& attribute : info.vertex_attributes) {
            creator.get_vertex_attribute_pusher()
                .location(attribute.location)
                .offset(attribute.offset)
                .format(attribute.format)
                .push();
        }

        return creator.color_attachment(rs_.display().swapchain().image_format())
            .depth_attachment(rs_.display().depth_img_view().format())
            .layout(info.layout)
            .shaders(info.vertex_shader, info.fragment_shader)
            .create();
    }

    [[nodiscard]] auto create_shader(const std::span<const char> code
    ) noexcept -> std::expected<hgn::shader_module, hgn::result> {
        return rs_.device().create_shader(code);
    }

    [[nodiscard]] auto create_descriptor_pool(const std::span<const hgn::descriptor_type> types
    ) noexcept -> std::expected<hgn::descriptor_pool, hgn::result> {
        return rs_.hard().device().create_descriptor_pool(types);
    }

    [[nodiscard]] auto create_descriptor_set_layout(
        const std::span<const descriptor_set_layout_binding> bindings
    ) noexcept -> std::expected<hgn::descriptor_set_layout, hgn::result> {
        auto creator = rs_.hard().device().get_descriptor_set_layout_creator();
        for (const auto& binding : bindings) {
            creator.get_binding_pusher()
                .binding(binding.binding)
                .type(binding.type)
                .shader_stage(binding.shader_stage)
                .push();
        }
        return creator.create();
    }

    [[nodiscard]] auto begin(
        const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a
    ) noexcept -> bool {
        return rs_.begin(win_.width(), win_.height(), r, g, b, a);
    }

    void bind_vertex_buffer(hgn::buffer& buffer) noexcept {
        rs_.command().buffer().bind_vertex_buffer(buffer);
    }

    void bind_index_buffer(hgn::buffer& buffer) noexcept {
        rs_.command().buffer().bind_index_buffer(buffer);
    }

    void bind_pipeline(hgn::graphics_pipeline& pipeline) noexcept {
        rs_.command().buffer().bind_pipeline(pipeline);
    }

    void bind_descriptor_set(hgn::pipeline_layout& layout, hgn::descriptor_set& set) noexcept {
        rs_.command().buffer().bind_descriptor_set(layout, set);
    }

    template <typename T>
    void push_constants(
        hgn::pipeline_layout&             layout,
        const hgn::shader_stage_flag_bits flag,
        std::uint32_t                     offset,
        std::uint32_t                     size,
        const T&                          constant
    ) noexcept {
        rs_.command().buffer().push_constants(layout, flag, offset, size, constant);
    }

    void draw(
        const std::uint32_t index_count,
        const std::uint32_t instance_count,
        const std::uint32_t first_index,
        const std::uint32_t vertex_offset
    ) noexcept {
        rs_.command().buffer().draw_indexed(
            index_count, instance_count, first_index, vertex_offset
        );
    }

    [[nodiscard]] auto end() noexcept -> bool { return rs_.end(win_.width(), win_.height()); }

  private:
    engine(kgm::window win, rendering_system rs) noexcept
        : win_{std::move(win)}, rs_{std::move(rs)} {}

    kgm::window      win_;
    rendering_system rs_;
};
}  // namespace sui