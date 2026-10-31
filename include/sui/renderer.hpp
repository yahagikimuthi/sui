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

#include <hgn/buffer.hpp>
#include <hgn/enums.hpp>
#include <hgn/physical.hpp>
#include <hgn/pipeline.hpp>
#include <hgn/pipeline_layout.hpp>
#include <hgn/pipeline_structs.hpp>
#include <hgn/shader.hpp>

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
        auto& dev    = hard.device();
        auto  buffer = dev.make_buffer(max_buffer_size);
        if (not buffer) return make_error(runtime_error, "Failed to make buffer.");

        const auto mem_requirements = buffer->get_memory_requirements();
        const auto memory_type      = find_memory_type(
            hard.physical(),
            mem_requirements.memory_type_bits(),
            hgn::memory_property_flags::host_visible | hgn::memory_property_flags::host_coherent
        );
        if (not memory_type) return make_error(runtime_error, "Failed to find memory type.");

        auto memory = dev.allocate_memory(mem_requirements.size(), *memory_type);
        if (not memory) return make_error(runtime_error, "Failed to allocate memory");

        if (not memory->bind_buffer(*buffer))
            return make_error(runtime_error, "Failed to bind buffer.");

        auto data = memory->map<max_buffer_size>();
        if (not data) return make_error(runtime_error, "Failed to map memory.");

        auto vert_source = read_file("../include/sui/vert.spv");
        if (not vert_source) return make_error(logic_error, "Failed to read vertex source file.");
        auto vert = dev.make_shader(*vert_source);
        if (not vert) return make_error(runtime_error, "Failed to make vertex shader module.");

        auto frag_source = read_file("../include/sui/frag.spv");
        if (not frag_source) return make_error(logic_error, "Failed to read fragment source file.");
        auto frag = dev.make_shader(*frag_source);
        if (not frag) return make_error(runtime_error, "Failed to make fragment shader module.");

        auto layout = dev.get_pipeline_layout();
        if (not layout) return make_error(runtime_error, "Failed to get pipeline layout");

        auto pipe = dev.get_graphics_pipeline_builder()
                        .vertex_binding(sizeof(vertex))
                        .vertex_offsets(offsetof(vertex, position), offsetof(vertex, color))
                        .color_attachment(fmt)
                        .layout(*layout)
                        .shaders(*vert, *frag)
                        .build();

        if (not pipe) return make_error(runtime_error, "Failed to graphics make pipeline.");

        return renderer{
            std::move(*buffer),
            std::move(*memory),
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
        hgn::buffer&&                         buffer,
        hgn::device_memory&&                  memory,
        std::span<std::byte, max_buffer_size> data,
        hgn::shader_module&&                  vert,
        hgn::shader_module&&                  frag,
        hgn::pipeline_layout&&                layout,
        hgn::graphics_pipeline&&              pipe
    ) noexcept
        : buffer_{std::move(buffer)},
          memory_{std::move(memory)},
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

    hgn::buffer                           buffer_;
    hgn::device_memory                    memory_;
    std::span<std::byte, max_buffer_size> data_;
    hgn::shader_module                    vert_;
    hgn::shader_module                    frag_;
    hgn::pipeline_layout                  layout_;
    hgn::graphics_pipeline                pipe_;
};
}  // namespace sui