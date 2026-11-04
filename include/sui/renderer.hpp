#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <optional>
#include <ranges>
#include <span>
#include <utility>

#include <hgn/allocator.hpp>
#include <hgn/buffer.hpp>
#include <hgn/enums.hpp>
#include <hgn/hgn.hpp>
#include <hgn/physical.hpp>
#include <hgn/rendering.hpp>

#include "sui/error.hpp"
#include "sui/hardware.hpp"
#include "sui/others.hpp"
#include "sui/vertex.hpp"

namespace sui {
class renderer final {
    static constexpr auto max_vertices    = 5000;
    static constexpr auto max_buffer_size = sizeof(vertex) * max_vertices;

  public:
    [[nodiscard]] static auto make(hardware& hard, const hgn::format fmt) noexcept
        -> std::expected<renderer, error> {
        auto allocator = hgn::get_allocator_creator()
                             .instance(hard.instance())
                             .physical(hard.physical())
                             .device(hard.device())
                             .create();
        if (not allocator) return make_error(runtime_error, "Failed to make allocator");

        auto& dev    = hard.device();
        auto  buffer = allocator->create_buffer(max_buffer_size);
        if (not buffer) return make_error(runtime_error, "Failed to make buffer.");

        // TODO プログラム終了時にunmapを呼び出して
        // RAIIクラスにラップするのもいいかも
        auto data = buffer->map();
        if (not data) return make_error(runtime_error, "Failed to map memory.");

        auto vert_source = read_file("../include/sui/vert.spv");
        if (not vert_source) return make_error(logic_error, "Failed to read vertex source file.");
        auto vert = dev.create_shader(*vert_source);
        if (not vert) return make_error(runtime_error, "Failed to make vertex shader module.");

        auto frag_source = read_file("../include/sui/frag.spv");
        if (not frag_source) return make_error(logic_error, "Failed to read fragment source file.");
        auto frag = dev.create_shader(*frag_source);
        if (not frag) return make_error(runtime_error, "Failed to make fragment shader module.");

        auto layout = dev.create_pipeline_layout();
        if (not layout) return make_error(runtime_error, "Failed to make pipeline layout");

        auto pipe = dev.get_graphics_pipeline_creator()
                        .vertex_binding(sizeof(vertex))
                        .vertex_offsets(offsetof(vertex, position), offsetof(vertex, color))
                        .color_attachment(fmt)
                        .layout(*layout)
                        .shaders(*vert, *frag)
                        .create();

        if (not pipe) return make_error(runtime_error, "Failed to make graphics pipeline.");

        return renderer{
            std::move(*allocator),
            std::move(*buffer),
            *data,
            std::move(*vert),
            std::move(*frag),
            std::move(*layout),
            std::move(*pipe)
        };
    }

    void update(const std::span<const vertex> vertices) noexcept {
        assert(vertices.size() <= max_vertices);
        std::memcpy(data_.data(), vertices.data(), sizeof(vertex) * vertices.size());
    }

    [[nodiscard]] auto buffer() noexcept -> hgn::buffer& { return buffer_; }
    [[nodiscard]] auto pipeline() noexcept -> hgn::graphics_pipeline& { return pipe_; }

  private:
    explicit renderer(
        hgn::allocator&&         allocator,
        hgn::buffer&&            buffer,
        std::span<std::byte>     data,
        hgn::shader_module&&     vert,
        hgn::shader_module&&     frag,
        hgn::pipeline_layout&&   layout,
        hgn::graphics_pipeline&& pipe
    ) noexcept
        : allocator_{std::move(allocator)},
          buffer_{std::move(buffer)},
          data_{data},
          vert_{std::move(vert)},
          frag_{std::move(frag)},
          layout_{std::move(layout)},
          pipe_{std::move(pipe)} {}

    [[nodiscard]] static auto find_memory_type(
        hgn::physical_device&            physical,
        const std::uint32_t              type_filter,
        const hgn::memory_property_flags properties
    ) noexcept -> std::optional<std::uint32_t> {
        const auto mem_properties = physical.get_memory_properties();
        const auto memory_types   = mem_properties.memory_types();

        for (const auto& [i, type] : std::views::enumerate(memory_types)) {
            if ((type_filter & (1 << i)) != 0 and
                (type.property_flags() & properties) == properties) {
                return i;
            }
        }

        return std::nullopt;
    }

    hgn::allocator         allocator_;
    hgn::buffer            buffer_;
    std::span<std::byte>   data_;
    hgn::shader_module     vert_;
    hgn::shader_module     frag_;
    hgn::pipeline_layout   layout_;
    hgn::graphics_pipeline pipe_;
};
}  // namespace sui