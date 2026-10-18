#pragma once

#include <cstdint>
#include <cstring>
#include <expected>
#include <functional>
#include <optional>
#include <span>
#include <utility>

#include <hgn/buffer.hpp>
#include <hgn/enums.hpp>
#include <hgn/physical_device.hpp>

#include "sui/error.hpp"

namespace sui {
class device_memory final {
  public:
    explicit device_memory(hgn::device_memory& native, hgn::device& dev) noexcept
        : native_{native}, dev_{dev} {}

    device_memory(const device_memory&) noexcept = delete;

    auto operator=(device_memory&) noexcept -> device_memory& = delete;

    device_memory(device_memory&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(device_memory&& other) noexcept -> device_memory& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;

        return *this;
    }

    ~device_memory() noexcept { destroy(); }

  private:
    void destroy() noexcept {
        if (native_) hgn::free_memory(dev_.get(), *native_);
    }

    std::optional<hgn::device_memory&>  native_;
    std::reference_wrapper<hgn::device> dev_;
};

class native_buffer final {
  public:
    explicit native_buffer(hgn::buffer& native, hgn::device& dev) noexcept
        : native_{native}, dev_{dev} {}

    native_buffer(const native_buffer&) noexcept = delete;

    auto operator=(const native_buffer&) noexcept -> native_buffer& = delete;

    native_buffer(native_buffer&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(native_buffer&& other) noexcept -> native_buffer& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;

        return *this;
    }

    ~native_buffer() noexcept { destroy(); }

  private:
    void destroy() noexcept {
        if (native_) hgn::destroy_buffer(dev_.get(), *native_);
    }

    std::optional<hgn::buffer&>         native_;
    std::reference_wrapper<hgn::device> dev_;
};

class buffer final {
  public:
    [[nodiscard]] static auto try_make(hgn::device& dev, hgn::physical_device& physical) noexcept
        -> std::expected<buffer, error> {
        constexpr auto max_buffer_size = std::uint64_t{sizeof(float) * 5000};

        auto info = hgn::buffer_create_info{};
        info.size(max_buffer_size)
            .usage(hgn::buffer_usage_flag_bits::vertex_buffer)
            .setting_sharing_mode(hgn::sharing_mode::exclusive);

        auto buf = hgn::try_make_buffer(dev, info);
        if (not buf) return make_error(runtime_error, "Failed to make buffer.");

        auto mem_requirements = hgn::get_buffer_memory_requirements(dev, *buf);

        auto alloc_info = hgn::memory_allocate_info{};
        alloc_info.allocation_size(mem_requirements.size());

        const auto memory_type = find_memory_type(
            physical,
            mem_requirements.memory_type_bits(),
            hgn::memory_property_flags::host_visible | hgn::memory_property_flags::host_coherent
        );
        if (not memory_type) return make_error(runtime_error, "Failed to find memory type.");

        alloc_info.memory_type_index(*memory_type);

        auto dev_mem = hgn::try_allocate_memory(dev, alloc_info);
        if (not dev_mem) return make_error(runtime_error, "Failed to allocate device memory.");

        if (not hgn::try_bind_buffer_memory(dev, buf->get(), dev_mem->get()))
            return make_error(runtime_error, "Failed to bind buffer and device memory.");

        auto data = hgn::try_map_memory(dev, dev_mem->get(), max_buffer_size);
        if (not data) return make_error(runtime_error, "Failed to map device memory.");

        return buffer{native_buffer{buf->get(), dev}, device_memory{dev_mem->get(), dev}, *data};
    }

    void update(const std::span<const float> vertices) noexcept {
        std::memcpy(mapped_data_, vertices.data(), sizeof(float) * vertices.size());
    }

  private:
    explicit buffer(native_buffer&& native, device_memory&& mem, void* mapped_data) noexcept
        : memory_{std::move(mem)}, buffer_{std::move(native)}, mapped_data_{mapped_data} {}

    [[nodiscard]] static auto find_memory_type(
        hgn::physical_device&            physical,
        const std::uint32_t              type_filter,
        const hgn::memory_property_flags properties
    ) noexcept -> std::optional<std::uint32_t> {
        auto mem_properties = hgn::get_physical_device_memory_properties(physical);

        for (const auto i : std::views::indices(mem_properties.memory_type_count())) {
            if ((type_filter & (1 << i)) != 0 and
                (mem_properties.setting_memory_types(i).property_flags() & properties) ==
                    properties) {
                return i;
            }
        }

        return std::nullopt;
    }

    device_memory memory_;
    native_buffer buffer_;
    void*         mapped_data_;
};
}  // namespace sui