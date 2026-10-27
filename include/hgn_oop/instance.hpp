#pragma once

#include <cstdint>
#include <expected>
#include <ranges>
#include <vector>

#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/instance.hpp>
#include <hgn/physical_device.hpp>
#include <hgn/surface.hpp>
#include <hgn_raii/device.hpp>
#include <hgn_raii/instance.hpp>
#include <hgn_raii/queue.hpp>
#include <hgn_raii/surface.hpp>

namespace hgn::oop {
class instance final {
  public:
    [[nodiscard]] static auto try_make(const hgn::instance_create_info& info) noexcept
        -> std::expected<instance, hgn::result> {
        auto native = hgn::raii::try_make_instance(info);
        if (not native) return std::unexpected{native.error()};
        return instance{std::move(*native)};
    }

    [[nodiscard]] auto get() noexcept -> hgn::raii::instance& { return native_; }

  private:
    explicit instance(hgn::raii::instance&& native) noexcept : native_{std::move(native)} {}

    hgn::raii::instance native_;
};

class surface_khr final {
  public:
    [[nodiscard]] static auto make(hgn::surface_khr&& native, instance& instance_ref) noexcept
        -> surface_khr {
        return surface_khr{hgn::raii::surface_khr::make(std::move(native), instance_ref.get())};
    }

    [[nodiscard]] auto get() noexcept -> hgn::raii::surface_khr& { return native_; }

  private:
    explicit surface_khr(hgn::raii::surface_khr&& native) noexcept : native_{std::move(native)} {}

    hgn::raii::surface_khr native_;
};

class physical_device final {
  public:
    [[nodiscard]] static auto try_enumerate(instance& instance_ref) noexcept
        -> std::expected<std::vector<physical_device>, hgn::result> {
        auto natives = hgn::raii::try_enumerate_physical(instance_ref.get());
        if (not natives) return std::unexpected{natives.error()};

        return *natives |
               std::views::transform(
                   [](const hgn::raii::physical_device& native) noexcept -> physical_device {
                       return physical_device{native};
                   }
               ) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto get_queue_family_properties() noexcept
        -> std::vector<hgn::queue_family_properties> {
        return hgn::get_physical_device_queue_family_properties(get());
    }

    [[nodiscard]] auto is_surface_supporting_khr(
        const std::uint32_t family_indices, surface_khr& surface
    ) noexcept -> bool {
        return hgn::is_physical_device_surface_support_khr(get(), family_indices, surface.get());
    }

    [[nodiscard]] auto get_memory_properties() noexcept -> physical_device_memory_properties {
        return hgn::get_physical_device_memory_properties(native_);
    }

    [[nodiscard]] auto get() noexcept -> raii::physical_device& { return native_; }

  private:
    explicit physical_device(const raii::physical_device& native) noexcept : native_{native} {}

    raii::physical_device native_;
};

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

  private:
    explicit device(raii::device&& native) noexcept : native_{std::move(native)} {}

    raii::device native_;
};
}  // namespace hgn::oop