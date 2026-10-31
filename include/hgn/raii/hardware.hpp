#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn::raii {
class instance final {
  public:
    [[nodiscard]] explicit instance(VkInstance_T&& raw) noexcept : raw_{raw} {}  // NOLINT

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

    [[nodiscard]] auto get() noexcept -> VkInstance_T& { return *raw_; }

    [[nodiscard]] operator VkInstance_T&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyInstance(&get(), nullptr);
    }

    std::optional<VkInstance_T&> raw_;
};

class surface_khr final {
  public:
    [[nodiscard]] explicit surface_khr(
        VkSurfaceKHR_T&& raw, VkInstance_T& instance  // NOLINT
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

    [[nodiscard]] auto get() noexcept -> VkSurfaceKHR_T& { return *raw_; }

    [[nodiscard]] operator VkSurfaceKHR_T&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroySurfaceKHR(&instance_.get(), &get(), nullptr);
    }
    std::optional<VkSurfaceKHR_T&>       raw_;
    std::reference_wrapper<VkInstance_T> instance_;
};

class device final {
  public:
    [[nodiscard]] explicit device(VkDevice_T&& raw) noexcept : raw_{raw} {}  // NOLINT

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

    [[nodiscard]] auto get() noexcept -> VkDevice_T& { return *raw_; }

    [[nodiscard]] operator VkDevice_T&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) {
            vkDestroyDevice(&get(), nullptr);
        }
    }

    std::optional<VkDevice_T&> raw_;
};

class physical_device final {
  public:
    [[nodiscard]] explicit physical_device(VkPhysicalDevice_T&& raw) noexcept  // NOLINT
        : raw_{raw} {}

    physical_device(const physical_device&) noexcept                    = delete;
    auto operator=(const physical_device&) noexcept -> physical_device& = delete;
    physical_device(physical_device&&) noexcept                         = default;
    auto operator=(physical_device&&) noexcept -> physical_device&      = default;
    ~physical_device() noexcept                                         = default;

    [[nodiscard]] auto get() noexcept -> VkPhysicalDevice_T& { return raw_; }
    [[nodiscard]] auto get() const noexcept -> VkPhysicalDevice_T& { return raw_; }

    [[nodiscard]] operator VkPhysicalDevice_T&() noexcept { return get(); }
    [[nodiscard]] operator VkPhysicalDevice_T&() const noexcept { return get(); }

  private:
    std::reference_wrapper<VkPhysicalDevice_T> raw_;
};

class queue final {
  public:
    [[nodiscard]] explicit queue(VkQueue_T&& raw) noexcept : raw_{raw} {}  // NOLINT

    queue(const queue&) noexcept                    = delete;
    auto operator=(const queue&) noexcept -> queue& = delete;
    queue(queue&&) noexcept                         = default;
    auto operator=(queue&&) noexcept -> queue&      = default;
    ~queue() noexcept                               = default;

    [[nodiscard]] auto get() noexcept -> VkQueue_T& { return raw_; }

    [[nodiscard]] operator VkQueue_T&() noexcept { return get(); }

  private:
    std::reference_wrapper<VkQueue_T> raw_;
};
}  // namespace hgn::raii