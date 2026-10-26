#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <optional>
#include <span>
#include <utility>

#include <hgn/buffer.hpp>
#include <hgn/enums.hpp>
#include <hgn/physical_device.hpp>

#include "sui/detail/hardware/hardware.hpp"
#include "sui/error.hpp"
#include "sui/vertex.hpp"

namespace sui::detail {
class device_memory final {
  public:
    explicit device_memory(hgn::device_memory&& native) noexcept : native_{std::move(native)} {}

    template <std::size_t N>
    [[nodiscard]] auto try_map_memory(hgn::device& dev) noexcept
        -> std::optional<std::span<std::byte, N>> {
        const auto res = hgn::try_map_memory<N>(dev, native_);
        if (not res) return std::nullopt;
        return *res;
    }

    [[nodiscard]] auto get() noexcept -> hgn::device_memory& { return native_; }

    [[nodiscard]] operator hgn::device_memory&() noexcept { return get(); }

  private:
    hgn::device_memory native_;
};

class native_buffer final {
  public:
    explicit native_buffer(hgn::buffer&& native) noexcept : native_{std::move(native)} {}

    [[nodiscard]] auto get_buffer_memory_requirements(hgn::device& dev) noexcept
        -> hgn::memory_requirements {
        return hgn::get_buffer_memory_requirements(dev, native_);
    }

    [[nodiscard]] auto get() noexcept -> hgn::buffer& { return native_; }

    [[nodiscard]] operator hgn::buffer&() noexcept { return get(); }

  private:
    hgn::buffer native_;
};

class buffer final {
    static constexpr auto max_vertices    = 5000;
    static constexpr auto max_buffer_size = sizeof(vertex) * max_vertices;

  public:
    [[nodiscard]] static auto try_make(hardware& hard) noexcept -> std::expected<buffer, error> {
        const auto info = hgn::buffer_create_info{}
                              .size(max_buffer_size)
                              .setting_sharing_mode(hgn::sharing_mode::exclusive);

        auto native_buf_res = hard.get_device().try_make_buffer(info);
        if (not native_buf_res) return make_error(runtime_error, "Failed to make buffer.");
        auto native_buf = native_buffer{std::move(*native_buf_res)};

        const auto mem_requirements = native_buf.get_buffer_memory_requirements(hard.get_device());

        const auto memory_type = find_memory_type(
            hard.get_physical(),
            mem_requirements.memory_type_bits(),
            hgn::memory_property_flags::host_visible | hgn::memory_property_flags::host_coherent
        );
        if (not memory_type) return make_error(runtime_error, "Failed to find memory type.");

        const auto alloc_info = hgn::memory_allocate_info{}
                                    .allocation_size(mem_requirements.size())
                                    .memory_type_index(*memory_type);

        auto dev_mem_res = hard.get_device().try_allocate_memory(alloc_info);
        if (not dev_mem_res) return make_error(runtime_error, "Failed to allocate device memory.");
        auto dev_mem = device_memory{std::move(*dev_mem_res)};

        if (not try_bind_buffer_memory(hard, native_buf, dev_mem))
            return make_error(runtime_error, "Failed to bind buffer and device memory.");

        const auto data = dev_mem.try_map_memory<max_buffer_size>(hard.get_device());
        if (not data) return make_error(runtime_error, "Failed to map device memory.");

        return buffer{std::move(native_buf), std::move(dev_mem), *data};
    }

    buffer(const buffer&) noexcept                    = delete;
    auto operator=(const buffer&) noexcept -> buffer& = delete;
    buffer(buffer&&) noexcept                         = default;
    auto operator=(buffer&&) noexcept -> buffer&      = default;
    ~buffer() noexcept                                = default;

    void update(const std::span<const vertex> vertices) noexcept {
        assert(vertices.size() <= max_vertices);
        std::memcpy(data_.data(), vertices.data(), sizeof(vertex) * vertices.size());
    }

    [[nodiscard]] auto get() noexcept -> hgn::buffer& { return buffer_.get(); }

  private:
    explicit buffer(
        native_buffer&& native, device_memory&& mem, std::span<std::byte, max_buffer_size> data
    ) noexcept
        : memory_{std::move(mem)}, buffer_{std::move(native)}, data_{data} {}

    [[nodiscard]] static auto find_memory_type(
        physical_device&                 physical,
        const std::uint32_t              type_filter,
        const hgn::memory_property_flags properties
    ) noexcept -> std::optional<std::uint32_t> {
        const auto mem_properties = physical.get_physical_device_memory_properties();
        for (const auto i : std::views::indices(mem_properties.memory_type_count())) {
            if ((type_filter & (1 << i)) != 0 and
                (mem_properties.setting_memory_type(i).property_flags() & properties) ==
                    properties) {
                return i;
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] static auto try_bind_buffer_memory(
        hardware& hard, native_buffer& buf, device_memory& dev_mem
    ) noexcept -> bool {
        return hgn::try_bind_buffer_memory(hard.get_device(), buf, dev_mem).has_value();
    }

    device_memory                         memory_;
    native_buffer                         buffer_;
    std::span<std::byte, max_buffer_size> data_;
};
}  // namespace sui::detail