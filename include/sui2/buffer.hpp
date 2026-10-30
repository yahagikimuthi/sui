#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <ranges>
#include <span>
#include <utility>

#include <hgn2/buffer.hpp>
#include <hgn2/enums.hpp>
#include <hgn2/physical.hpp>
#include <hgn2/shader.hpp>

#include "sui2/error.hpp"
#include "sui2/hardware.hpp"
#include "sui2/others.hpp"

namespace sui {
class renderer final {
    static constexpr auto max_vertices    = 5000;
    static constexpr auto max_buffer_size = sizeof(float) * 5 * max_vertices;

  public:
    [[nodiscard]] static auto make(hardware& hard) noexcept -> std::expected<renderer, error> {
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

        auto vert_source = read_file("../include/sui2/vert.spv");
        if (not vert_source) return make_error(logic_error, "Failed to read vertex source file.");
        auto vert = dev.make_shader(*vert_source);
        if (not vert) return make_error(runtime_error, "Failed to make vertex shader module.");

        auto frag_source = read_file("../include/sui2/frag.spv");
        if (not frag_source) return make_error(logic_error, "Failed to read fragment source file.");
        auto frag = dev.make_shader(*frag_source);
        if (not frag) return make_error(runtime_error, "Failed to make fragment shader module.");

        return sui::renderer{
            std::move(*buffer), std::move(*memory), *data, std::move(*vert), std::move(*frag)
        };
    }

  private:
    explicit renderer(
        hgn::buffer&&                         buffer,
        hgn::device_memory&&                  memory,
        std::span<std::byte, max_buffer_size> data,
        hgn::shader_module&&                  vert,
        hgn::shader_module&&                  frag
    ) noexcept
        : buffer_{std::move(buffer)},
          memory_{std::move(memory)},
          data_{data},
          vert_{std::move(vert)},
          frag_{std::move(frag)} {}

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
};
}  // namespace sui