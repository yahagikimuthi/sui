#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <ranges>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn2/enums.hpp"
#include "hgn2/instance.hpp"
#include "hgn2/surface.hpp"

namespace hgn {
class queue_family_properties final {
  public:
    [[nodiscard]] auto flags() const noexcept -> queue_flag_bits {
        return static_cast<queue_flag_bits>(raw_.queueFlags);
    }

  private:
    VkQueueFamilyProperties raw_;
};

class physical_device final {
    friend class instance;

  public:
    physical_device(const physical_device&) noexcept                    = delete;
    auto operator=(const physical_device&) noexcept -> physical_device& = delete;
    physical_device(physical_device&&) noexcept                         = default;
    auto operator=(physical_device&&) noexcept -> physical_device&      = default;
    ~physical_device() noexcept                                         = default;

    [[nodiscard]] auto name() const noexcept -> std::string {
        const auto property = get_property();
        return std::string{static_cast<const char*>(property.deviceName)};
    }

    [[nodiscard]] auto get_queue_family_properties() noexcept
        -> std::vector<queue_family_properties> {
        auto count = std::uint32_t{};
        vkGetPhysicalDeviceQueueFamilyProperties(&get(), &count, nullptr);

        auto properties = std::vector<VkQueueFamilyProperties>(count);
        vkGetPhysicalDeviceQueueFamilyProperties(&get(), &count, properties.data());

        return properties |
               std::views::transform(
                   [](const VkQueueFamilyProperties& property) noexcept -> queue_family_properties {
                       return std::bit_cast<queue_family_properties>(property);
                   }
               ) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto is_supporting_surface_khr(
        const std::uint32_t queue_family_index, surface_khr& surface
    ) noexcept -> std::expected<bool, result> {
        auto       supported = VkBool32{};
        const auto res       = vkGetPhysicalDeviceSurfaceSupportKHR(
            &get(), queue_family_index, &surface.get(), &supported
        );
        if (res != VK_SUCCESS) return make_unexpected(res);

        return supported != VK_FALSE;
    }

    [[nodiscard]] auto get() noexcept -> VkPhysicalDevice_T& { return raw_; }

    [[nodiscard]] auto get() const noexcept -> VkPhysicalDevice_T& { return raw_; }

  private:
    explicit physical_device(VkPhysicalDevice_T& raw) noexcept : raw_{std::ref(raw)} {}

    [[nodiscard]] auto get_property() const noexcept -> VkPhysicalDeviceProperties {
        auto property = VkPhysicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(&get(), &property);
        return property;
    }

    std::reference_wrapper<VkPhysicalDevice_T> raw_;
};

inline auto instance::enumerate_physical() noexcept
    -> std::expected<std::vector<physical_device>, result> {
    auto count = std::uint32_t{};
    if (const auto res = vkEnumeratePhysicalDevices(&get(), &count, nullptr); res != VK_SUCCESS) {
        return make_unexpected(res);
    }

    auto devices = std::vector<VkPhysicalDevice_T*>(count);
    if (const auto res = vkEnumeratePhysicalDevices(&get(), &count, devices.data());
        res != VK_SUCCESS) {
        return make_unexpected(res);
    }

    return devices | std::views::transform([](VkPhysicalDevice_T* ptr) noexcept -> physical_device {
               assert(ptr != nullptr);
               return physical_device{*ptr};
           }) |
           std::ranges::to<std::vector>();
}
}  // namespace hgn