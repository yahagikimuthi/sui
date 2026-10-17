#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/buffer.hpp>
#include <hgn/enums.hpp>
#include <hgn/physical_device.hpp>

#include "sui/error.hpp"

namespace sui {
class device_memory final {
  public:
    [[nodiscard]] static auto try_make(hgn::device& dev) noexcept
        -> std::expected<device_memory, error>;

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
    explicit device_memory(hgn::device_memory& native, hgn::device& dev) noexcept
        : native_{native}, dev_{dev} {}

    void destroy() noexcept {
        if (native_) hgn::free_memory(dev_.get(), *native_);
    }

    std::optional<hgn::device_memory&>  native_;
    std::reference_wrapper<hgn::device> dev_;
};

class buffer final {
  public:
    [[nodiscard]] static auto try_make(
        hgn::device& dev, hgn::physical_device& physical, hgn::device_memory& dev_mem
    ) noexcept -> std::expected<buffer, error> {
        constexpr auto max_buffer_size = std::uint64_t{sizeof(float) * 5000};

        auto info = hgn::buffer_create_info{};
        info.size(max_buffer_size)
            .usage(hgn::buffer_usage_flag_bits::vertex_buffer)
            .setting_sharing_mode(hgn::sharing_mode::exclusive);

        auto native = hgn::try_make_buffer(dev, info);
        if (not native) return make_error(runtime_error, "Failed to make buffer.");

        auto mem_requirements = hgn::get_buffer_memory_requirements(dev, *native);

        auto alloc_info = hgn::memory_allocate_info{};
        alloc_info.allocation_size(mem_requirements.size());

        const auto memory_type = find_memory_type(
            physical,
            mem_requirements.memory_type_bits(),
            hgn::memory_property_flags::host_visible | hgn::memory_property_flags::host_coherent
        );
        if (not memory_type) return make_error(runtime_error, "Failed to find memory type.");

        alloc_info.memory_type_index(*memory_type);

        if (not hgn::try_allocate_memory(dev, alloc_info, dev_mem))
            return make_error(runtime_error, "Failed to allocate memory.");
    }

    buffer(const buffer&) noexcept = delete;

    auto operator=(const buffer&) noexcept -> buffer& = delete;

    buffer(buffer&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(buffer&& other) noexcept -> buffer& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;

        return *this;
    }

    ~buffer() noexcept { destroy(); }

  private:
    explicit buffer(hgn::buffer& native, hgn::device& dev) noexcept : native_{native}, dev_{dev} {}

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

    void destroy() noexcept {
        if (native_) hgn::destroy_buffer(dev_.get(), *native_);
    }

    std::optional<hgn::buffer&>         native_;
    std::reference_wrapper<hgn::device> dev_;
};
}  // namespace sui