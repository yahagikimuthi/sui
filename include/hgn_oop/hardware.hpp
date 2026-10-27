#pragma once

#include <cstdint>
#include <expected>
#include <ranges>
#include <utility>
#include <vector>

#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/instance.hpp>
#include <hgn/physical_device.hpp>
#include <hgn/submit.hpp>
#include <hgn/surface.hpp>
#include <hgn/sync.hpp>
#include <hgn_raii/device.hpp>
#include <hgn_raii/instance.hpp>
#include <hgn_raii/queue.hpp>
#include <hgn_raii/surface.hpp>
#include <hgn_raii/sync.hpp>

#include "hgn_oop/sync.hpp"

namespace hgn::oop {
class instance final {
  public:
    [[nodiscard]] static auto try_make(const hgn::instance_create_info& info) noexcept
        -> std::expected<oop::instance, hgn::result> {
        auto native = hgn::raii::try_make_instance(info);
        if (not native) return std::unexpected{native.error()};
        return oop::instance{std::move(*native)};
    }

    [[nodiscard]] auto get() noexcept -> raii::instance& { return native_; }

  private:
    explicit instance(raii::instance&& native) noexcept : native_{std::move(native)} {}

    raii::instance native_;
};

[[nodiscard]] inline auto try_make_instance(const hgn::instance_create_info& info) noexcept
    -> std::expected<oop::instance, hgn::result> {
    return oop::instance::try_make(info);
}

class surface_khr final {
  public:
    [[nodiscard]] static auto make(hgn::surface_khr&& native, oop::instance& instance_ref) noexcept
        -> oop::surface_khr {
        return oop::surface_khr{raii::surface_khr::make(std::move(native), instance_ref.get())};
    }

    [[nodiscard]] auto get() noexcept -> raii::surface_khr& { return native_; }

  private:
    explicit surface_khr(raii::surface_khr&& native) noexcept : native_{std::move(native)} {}

    raii::surface_khr native_;
};

[[nodiscard]] inline auto make_surface(
    hgn::surface_khr&& native, oop::instance& instance_ref
) noexcept -> oop::surface_khr {
    return oop::surface_khr::make(std::move(native), instance_ref);
}

class physical_device final {
  public:
    [[nodiscard]] static auto try_enumerate(oop::instance& instance_ref) noexcept
        -> std::expected<std::vector<oop::physical_device>, hgn::result> {
        auto natives = raii::try_enumerate_physical(instance_ref.get());
        if (not natives) return std::unexpected{natives.error()};

        return *natives |
               std::views::transform(
                   [](const raii::physical_device& native) noexcept -> oop::physical_device {
                       return oop::physical_device{native};
                   }
               ) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto get_properties() noexcept -> hgn::physical_device_properties {
        return hgn::get_physical_device_properties(native_);
    }

    [[nodiscard]] auto get_queue_family_properties() noexcept
        -> std::vector<hgn::queue_family_properties> {
        return hgn::get_physical_device_queue_family_properties(get());
    }

    [[nodiscard]] auto is_surface_supporting_khr(
        const std::uint32_t family_indices, oop::surface_khr& surface
    ) noexcept -> bool {
        return hgn::is_physical_device_surface_supporting_khr(get(), family_indices, surface.get());
    }

    [[nodiscard]] auto get_memory_properties() noexcept -> hgn::physical_device_memory_properties {
        return hgn::get_physical_device_memory_properties(native_);
    }

    [[nodiscard]] auto try_get_surface_capabilities_khr(oop::surface_khr& surface) noexcept
        -> std::expected<hgn::surface_capabilities_khr, hgn::result> {
        return hgn::try_get_physical_device_surface_capabilities_khr(native_, surface.get());
    }

    [[nodiscard]] auto try_get_surface_formats(oop::surface_khr& surface) noexcept
        -> std::expected<std::vector<hgn::surface_format_khr>, hgn::result> {
        return hgn::try_get_physical_device_surface_formats_khr(native_, surface.get());
    }

    [[nodiscard]] auto try_get_surface_present_modes(oop::surface_khr& surface) noexcept
        -> std::expected<std::vector<hgn::present_mode_khr>, hgn::result> {
        return hgn::try_get_physical_device_surface_present_modes_khr(native_, surface.get());
    }

    [[nodiscard]] auto get() noexcept -> raii::physical_device& { return native_; }

  private:
    explicit physical_device(const raii::physical_device& native) noexcept : native_{native} {}

    raii::physical_device native_;
};

[[nodiscard]] inline auto try_enumerate_physical(oop::instance& instance_ref) noexcept
    -> std::expected<std::vector<oop::physical_device>, hgn::result> {
    return oop::physical_device::try_enumerate(instance_ref);
}

class device final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::device_create_info& info, oop::physical_device& physical
    ) noexcept -> std::expected<oop::device, hgn::result> {
        auto native = hgn::raii::try_make_device(info, physical.get());
        if (not native) return std::unexpected{native.error()};
        return oop::device{std::move(*native)};
    }

    [[nodiscard]] auto get() noexcept -> raii::device& { return native_; }

    [[nodiscard]] auto try_make_semaphore(const hgn::semaphore_create_info& info) noexcept
        -> std::expected<oop::semaphore, hgn::result> {
        auto native = hgn::raii::try_make_semaphore(info, native_);
        if (not native) return std::unexpected{native.error()};

        return oop::semaphore{std::move(*native)};
    }

    [[nodiscard]] auto try_make_fence(const hgn::fence_create_info& info) noexcept
        -> std::expected<oop::fence, hgn::result> {
        auto native = hgn::raii::try_make_fence(info, native_);
        if (not native) return std::unexpected{native.error()};

        return oop::fence{std::move(*native)};
    }

  private:
    explicit device(raii::device&& native) noexcept : native_{std::move(native)} {}

    raii::device native_;
};

[[nodiscard]] inline auto try_make_device(
    const hgn::device_create_info& info, oop::physical_device& physical
) noexcept -> std::expected<oop::device, hgn::result> {
    return oop::device::try_make(info, physical);
}

class queue final {
  public:
    [[nodiscard]] static auto try_make(
        oop::device& dev, const std::uint32_t family_indices
    ) noexcept -> std::optional<queue> {
        auto native = raii::try_get_queue(dev.get(), family_indices);
        if (not native) return std::nullopt;
        return oop::queue{*native};
    }

    [[nodiscard]] auto try_submit(const hgn::submit_info& info, oop::fence& fence_ref) noexcept
        -> std::expected<void, hgn::result> {
        return hgn::try_queue_submit(native_, info, fence_ref.get());
    }

    [[nodiscard]] auto get() noexcept -> raii::queue& { return native_; }

  private:
    explicit queue(const raii::queue& native) noexcept : native_{native} {}

    raii::queue native_;
};

[[nodiscard]] inline auto try_make_queue(
    oop::device& dev, const std::uint32_t family_indices
) noexcept -> std::optional<queue> {
    return oop::queue::try_make(dev, family_indices);
}
}  // namespace hgn::oop