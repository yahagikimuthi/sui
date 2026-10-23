#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/command.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <hgn/render.hpp>

#include "sui/error.hpp"

namespace sui {
class command_pool final {
  public:
    [[nodiscard]] static auto try_make(
        hgn::device& dev, const std::uint32_t queue_family_indices
    ) noexcept -> std::expected<command_pool, error> {
        auto info = hgn::command_pool_create_info{};
        info.queue_family_index(queue_family_indices);

        auto res = hgn::try_make_command_pool(dev, info);
        if (not res) return make_error(runtime_error, "Failed to make command pool.");

        return command_pool{*res, dev};
    }

    command_pool(const command_pool&) noexcept = delete;

    auto operator=(const command_pool&) noexcept -> command_pool& = delete;

    command_pool(command_pool&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(command_pool&& other) noexcept -> command_pool& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;

        return *this;
    }

    ~command_pool() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::command_pool& { return *native_; }

  private:
    explicit command_pool(hgn::command_pool& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_command_pool(dev_.get(), *native_);
    }

    std::optional<hgn::command_pool&>   native_;
    std::reference_wrapper<hgn::device> dev_;
};

class command_buffer final {
  public:
    [[nodiscard]] static auto try_make(hgn::device& dev, hgn::command_pool& pool) noexcept
        -> std::expected<command_buffer, error> {
        auto alloc_info = hgn::command_buffer_allocate_info{};
        alloc_info.level(hgn::command_buffer_level::primary)
            .command_buffer_count(1)
            .command_pool(pool);
        auto res = hgn::try_allocate_command_buffers(dev, alloc_info);

        if (not res) return make_error(runtime_error, "Failed to make command buffer.");

        return command_buffer{*res};
    }

    [[nodiscard]] auto try_begin_rendering(hgn::image& image, hgn::image_view& view) noexcept
        -> bool {
        auto begin_info = hgn::command_buffer_begin_info{};
        auto res        = hgn::try_begin_command_buffer(native_.get(), begin_info);
        if (not res) return false;

        auto barrier = hgn::image_memory_barrier{};
        barrier.layout(hgn::image_layout::undefined, hgn::image_layout::color_attachment_optimal)
            .setting_image(image);

        hgn::command_pipeline_barrier(native_.get(), barrier);

        auto color_attachment = hgn::rendering_attachment_info{};
        color_attachment.setting_image_view(view).clear_color(0.1f, 0.2f, 0.4f, 1.f);

        auto render_info = hgn::rendering_info{};
        render_info.extent(800, 600).color_attachment(color_attachment);

        hgn::command_begin_rendering(native_.get(), render_info);
        return true;
    }

    [[nodiscard]] auto try_end_rendering(hgn::image& image) noexcept -> bool {
        hgn::command_end_rendering(native_.get());

        auto barrier = hgn::image_memory_barrier{};
        barrier
            .layout(hgn::image_layout::color_attachment_optimal, hgn::image_layout::present_src_khr)
            .setting_image(image);

        hgn::command_pipeline_barrier(native_.get(), barrier);

        auto res = hgn::try_end_command_buffer(native_.get());
        return res.has_value();
    }

    [[nodiscard]] auto get() noexcept -> hgn::command_buffer& { return native_.get(); }

  private:
    explicit command_buffer(hgn::command_buffer& native) noexcept : native_{native} {}

    std::reference_wrapper<hgn::command_buffer> native_;
};

class command final {
  public:
    [[nodiscard]] static auto try_make(
        hgn::device& dev, const std::uint32_t queue_family_indices
    ) noexcept -> std::expected<command, error> {
        auto pool = command_pool::try_make(dev, queue_family_indices);
        if (not pool)
            return make_error(runtime_error, "Failed to make command pool.", pool.error());

        auto buffer = command_buffer::try_make(dev, pool->get());
        if (not buffer)
            return make_error(runtime_error, "Failed to make command buffer.", buffer.error());

        return command{std::move(*pool), *buffer};
    }

    command(const command&) noexcept                    = delete;
    auto operator=(const command&) noexcept -> command& = delete;
    command(command&&) noexcept                         = default;
    auto operator=(command&&) noexcept -> command&      = default;
    ~command() noexcept                                 = default;

    [[nodiscard]] auto try_begin_rendering(hgn::image& image, hgn::image_view& view) noexcept
        -> bool {
        return buffer_.try_begin_rendering(image, view);
    }

    [[nodiscard]] auto try_end_rendering(hgn::image& image) noexcept -> bool {
        return buffer_.try_end_rendering(image);
    }

    [[nodiscard]] auto buffer() noexcept -> hgn::command_buffer& { return buffer_.get(); }

  private:
    explicit command(command_pool&& pool, const command_buffer& buffer) noexcept
        : pool_{std::move(pool)}, buffer_{buffer} {}

    command_pool   pool_;
    command_buffer buffer_;
};
}  // namespace sui