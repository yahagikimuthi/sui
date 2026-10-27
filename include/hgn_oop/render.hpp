#pragma once

#include <cstdint>
#include <expected>
#include <utility>

#include <hgn/buffer.hpp>
#include <hgn/enums.hpp>
#include <hgn/pipeline.hpp>
#include <hgn_oop/hardware.hpp>
#include <hgn_raii/buffer.hpp>
#include <hgn_raii/pipeline.hpp>

namespace hgn::oop {
class pipeline_layout final {
  public:
    [[nodiscard]] static auto try_make(
        hgn::pipeline_layout_create_info& info, oop::device& dev
    ) noexcept -> std::expected<oop::pipeline_layout, hgn::result> {
        auto native = raii::try_make_pipeline_layout(info, dev.get());
        if (not native) return std::unexpected{native.error()};

        return oop::pipeline_layout{std::move(*native)};
    }

  private:
    explicit pipeline_layout(raii::pipeline_layout&& native) noexcept
        : native_{std::move(native)} {}

    raii::pipeline_layout native_;
};

class graphics_pipeline final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::graphics_pipeline_create_info& info, oop::device& dev
    ) noexcept -> std::expected<oop::graphics_pipeline, hgn::result> {
        auto native = raii::try_make_graphics_pipeline(info, dev.get());
        if (not native) return std::unexpected{native.error()};

        return oop::graphics_pipeline{std::move(*native)};
    }

    [[nodiscard]] auto get() noexcept -> raii::graphics_pipeline& { return native_; }

  private:
    explicit graphics_pipeline(raii::graphics_pipeline&& native) noexcept
        : native_{std::move(native)} {}

    raii::graphics_pipeline native_;
};

class buffer final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::buffer_create_info& info, oop::device& dev
    ) noexcept -> std::expected<oop::buffer, hgn::result> {
        auto native = raii::try_make_buffer(info, dev.get());
        if (not native) return std::unexpected{native.error()};

        return buffer{std::move(*native)};
    }

    [[nodiscard]] auto get_memory_requirements(oop::device& dev) noexcept
        -> hgn::memory_requirements {
        return hgn::get_buffer_memory_requirements(dev.get(), native_);
    }

    [[nodiscard]] auto get() noexcept -> raii::buffer& { return native_; }

  private:
    explicit buffer(raii::buffer&& native) noexcept : native_{std::move(native)} {}

    raii::buffer native_;
};

class device_memory final {
  public:
    [[nodiscard]] static auto try_allocate(
        const hgn::memory_allocate_info& info, oop::device& dev
    ) noexcept -> std::expected<oop::device_memory, hgn::result> {
        auto native = raii::try_allocate_memory(info, dev.get());
        if (not native) return std::unexpected{native.error()};

        return device_memory{std::move(*native)};
    }

    [[nodiscard]] auto try_bind_buffer(oop::buffer& buf, oop::device& dev) noexcept
        -> std::expected<void, hgn::result> {
        return hgn::try_bind_buffer_memory(dev.get(), buf.get(), native_);
    }

    template <std::uint64_t N>
    [[nodiscard]] auto try_map(oop::device& dev) noexcept
        -> std::expected<std::span<std::byte, N>, hgn::result> {
        return hgn::try_map_memory<N>(dev.get(), native_);
    }

  private:
    explicit device_memory(raii::device_memory&& native) noexcept : native_{std::move(native)} {}

    raii::device_memory native_;
};
}  // namespace hgn::oop