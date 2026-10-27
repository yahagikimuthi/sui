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
#include <hgn/surface.hpp>
#include <hgn_raii/device.hpp>
#include <hgn_raii/instance.hpp>
#include <hgn_raii/surface.hpp>

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