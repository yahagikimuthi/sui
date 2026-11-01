#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raw/raw.hpp"

namespace hgn::raii {
class instance final {
  public:
    [[nodiscard]] explicit instance(raw::instance&& raw) noexcept : raw_{raw} {}  // NOLINT

    instance(const instance& other) noexcept                    = delete;
    auto operator=(const instance& other) noexcept -> instance& = delete;

    instance(instance&& other) noexcept : raw_{std::exchange(other.raw_, std::nullopt)} {}

    auto operator=(instance&& other) noexcept -> instance& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        return *this;
    }

    ~instance() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> raw::instance& { return *raw_; }

    [[nodiscard]] operator raw::instance&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyInstance(&get(), nullptr);
    }

    std::optional<raw::instance&> raw_;
};

class surface_khr final {
  public:
    [[nodiscard]] explicit surface_khr(
        raw::surface_khr&& raw, raw::instance& instance  // NOLINT
    ) noexcept
        : raw_{raw}, instance_{instance} {}

    surface_khr(const surface_khr&) noexcept                    = delete;
    auto operator=(const surface_khr&) noexcept -> surface_khr& = delete;

    surface_khr(surface_khr&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, instance_{other.instance_} {}

    auto operator=(surface_khr&& other) noexcept -> surface_khr& {
        if (this == &other) return *this;

        destroy();

        raw_      = std::exchange(other.raw_, std::nullopt);
        instance_ = other.instance_;
        return *this;
    }

    ~surface_khr() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> raw::surface_khr& { return *raw_; }

    [[nodiscard]] operator raw::surface_khr&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroySurfaceKHR(&instance_.get(), &get(), nullptr);
    }
    std::optional<raw::surface_khr&>      raw_;
    std::reference_wrapper<raw::instance> instance_;
};

class device final {
  public:
    [[nodiscard]] explicit device(raw::device&& raw) noexcept : raw_{raw} {}  // NOLINT

    device(const device&) noexcept                    = delete;
    auto operator=(const device&) noexcept -> device& = delete;

    device(device&& other) noexcept : raw_{std::exchange(other.raw_, std::nullopt)} {}

    auto operator=(device&& other) noexcept -> device& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        return *this;
    }

    ~device() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> raw::device& { return *raw_; }

    [[nodiscard]] operator raw::device&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) {
            vkDestroyDevice(&get(), nullptr);
        }
    }

    std::optional<raw::device&> raw_;
};

class physical_device final {
  public:
    [[nodiscard]] explicit physical_device(raw::physical_device&& raw) noexcept  // NOLINT
        : raw_{raw} {}

    [[nodiscard]] auto get() noexcept -> raw::physical_device& { return raw_; }
    [[nodiscard]] auto get() const noexcept -> raw::physical_device& { return raw_; }

    [[nodiscard]] operator raw::physical_device&() noexcept { return get(); }
    [[nodiscard]] operator raw::physical_device&() const noexcept { return get(); }

  private:
    std::reference_wrapper<raw::physical_device> raw_;
};

class queue_family final {
  public:
    explicit queue_family(std::uint32_t index, raw::physical_device& physical) noexcept
        : index_{index}, physical_{std::ref(physical)} {}

    [[nodiscard]] auto index() const noexcept -> std::uint32_t { return index_; }

    [[nodiscard]] auto init_physical() noexcept -> raw::physical_device& { return physical_; }

  private:
    std::uint32_t                                index_;
    std::reference_wrapper<raw::physical_device> physical_;
};

class queue final {
  public:
    [[nodiscard]] explicit queue(raw::queue&& raw) noexcept : raw_{raw} {}  // NOLINT

    [[nodiscard]] auto get() noexcept -> raw::queue& { return raw_; }

    [[nodiscard]] operator raw::queue&() noexcept { return get(); }

  private:
    std::reference_wrapper<raw::queue> raw_;
};
}  // namespace hgn::raii