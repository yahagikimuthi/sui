#pragma once

#include <expected>
#include <optional>
#include <utility>

#include <hgn/buffer.hpp>
#include <hgn/command.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/pipeline.hpp>
#include <hgn/shader.hpp>
#include <hgn/swapchain.hpp>
#include <hgn/sync.hpp>

#include "hgn/image.hpp"
#include "sui/detail/hardware/physical_device.hpp"
#include "sui/detail/others.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class device final {
  public:
    [[nodiscard]] static auto try_make(physical_device& physical) noexcept
        -> std::expected<device, error> {
        const auto queue_info =
            hgn::device_queue_create_info{}.queue_family_index(physical.graphics_family());

        const auto info =
            hgn::device_create_info{}
                .queue_create_info(queue_info)
                .features(hgn::physical_device_features{})
                .extensions({hgn::khr_swapchain_extention_name})
                .dynamic_rendering_features(hgn::physical_device_dynamic_rendering_features{});

        auto res = physical.try_make_device(info);
        if (not res) return make_error(runtime_error, "Failed to make logical device\n");
        return device{std::move(*res)};
    }

    device(const device&) noexcept                    = delete;
    auto operator=(const device&) noexcept -> device& = delete;
    device(device&&) noexcept                         = default;
    auto operator=(device&&) noexcept -> device&      = default;
    ~device() noexcept                                = default;

    [[nodiscard]] auto try_wait_idle() noexcept -> bool {
        const auto res = hgn::try_device_wait_idle(native_);
        return res.has_value();
    }

    [[nodiscard]] auto try_make_buffer(const hgn::buffer_create_info& info) noexcept
        -> std::optional<hgn::buffer> {
        auto res = hgn::try_make_buffer(native_, info);
        return to_optional(std::move(res));
    }

    [[nodiscard]] auto try_allocate_memory(const hgn::memory_allocate_info& info) noexcept
        -> std::optional<hgn::device_memory> {
        auto res = hgn::try_allocate_memory(native_, info);
        return to_optional(std::move(res));
    }

    [[nodiscard]] auto try_make_pipeline_layout(
        const hgn::pipeline_layout_create_info& info
    ) noexcept -> std::optional<hgn::pipeline_layout&> {
        auto res = hgn::try_make_pipeline_layout(native_, info);
        return to_optional(res);
    }

    [[nodiscard]] auto try_make_shader_module(const hgn::shader_module_create_info& info) noexcept
        -> std::optional<hgn::shader_module&> {
        auto res = hgn::try_make_shader_module(native_, info);
        return to_optional(res);
    }

    [[nodiscard]] auto try_make_swapchain(const hgn::swapchain_create_info_khr& info) noexcept
        -> std::optional<hgn::swapchain_khr&> {
        auto res = hgn::try_make_swapchain(native_, info);
        return to_optional(res);
    }

    [[nodiscard]] auto try_make_semaphore(const hgn::semaphore_create_info& info) noexcept
        -> std::optional<hgn::semaphore&> {
        auto res = hgn::try_make_semaphore(native_, info);
        return to_optional(res);
    }

    [[nodiscard]] auto try_make_fence(const hgn::fence_create_info& info) noexcept
        -> std::optional<hgn::fence&> {
        auto res = hgn::try_make_fence(native_, info);
        return to_optional(res);
    }

    [[nodiscard]] auto try_allocate_command_buffer(
        const hgn::command_buffer_allocate_info& info
    ) noexcept -> std::optional<hgn::command_buffer&> {
        auto res = hgn::try_allocate_command_buffers(native_, info);
        return to_optional(res);
    }

    [[nodiscard]] auto try_get_device_queue(const std::uint32_t family_indices) noexcept
        -> std::optional<hgn::queue&> {
        return hgn::try_get_device_queue(native_, family_indices);
    }

    [[nodiscard]] auto try_make_command_pool(const hgn::command_pool_create_info& info) noexcept
        -> std::optional<hgn::command_pool&> {
        auto res = hgn::try_make_command_pool(native_, info);
        return to_optional(res);
    }

    [[nodiscard]] auto try_make_image_view(const hgn::image_view_create_info& info) noexcept
        -> std::optional<hgn::image_view&> {
        auto res = hgn::try_make_image_view(native_, info);
        return to_optional(res);
    }

    [[nodiscard]] operator hgn::device&() noexcept { return native_; }

  private:
    explicit device(hgn::device&& native) noexcept : native_{std::move(native)} {}

    hgn::device native_;
};
}  // namespace sui::detail