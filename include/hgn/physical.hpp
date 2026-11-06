#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/queue_family.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/structs.hpp"
#include "hgn/surface.hpp"

namespace hgn {
class device;
class physical_device final {
    friend class instance;
    friend class device;

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

    [[nodiscard]] auto get_queue_families() noexcept -> std::vector<queue_family> {
        auto count = std::uint32_t{};
        vkGetPhysicalDeviceQueueFamilyProperties(&get(), &count, nullptr);

        return std::views::indices(count) |
               std::views::transform([this](std::uint32_t index) noexcept -> queue_family {
                   return queue_family{index, get()};
               }) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto get_surface_capabilities(surface_khr& surface
    ) noexcept -> std::expected<surface_capabilities_khr, result> {
        auto       capabilities = VkSurfaceCapabilitiesKHR{};  // NOLINT
        const auto res =
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(&get(), &surface.get(), &capabilities);
        if (res != VK_SUCCESS) return make_unexpected(res);

        return std::bit_cast<surface_capabilities_khr>(capabilities);
    }

    [[nodiscard]] auto get_surface_formats(surface_khr& surface
    ) noexcept -> std::expected<std::vector<surface_format_khr>, result> {
        auto       count = std::uint32_t{};
        const auto res1 =
            vkGetPhysicalDeviceSurfaceFormatsKHR(&get(), &surface.get(), &count, nullptr);
        if (res1 != VK_SUCCESS) return make_unexpected(res1);

        auto       formats = std::vector<VkSurfaceFormatKHR>(count);
        const auto res2 =
            vkGetPhysicalDeviceSurfaceFormatsKHR(&get(), &surface.get(), &count, formats.data());
        if (res2 != VK_SUCCESS) return make_unexpected(res2);

        return formats |
               std::views::transform(
                   [](const VkSurfaceFormatKHR& raw) noexcept -> surface_format_khr {
                       return std::bit_cast<surface_format_khr>(raw);
                   }
               ) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto get_surface_present_modes(surface_khr& surface
    ) noexcept -> std::expected<std::vector<present_mode_khr>, result> {
        auto       count = std::uint32_t{};
        const auto res1 =
            vkGetPhysicalDeviceSurfacePresentModesKHR(&get(), &surface.get(), &count, nullptr);
        if (res1 != VK_SUCCESS) return make_unexpected(res1);

        auto       modes = std::vector<VkPresentModeKHR>(count);
        const auto res2 =
            vkGetPhysicalDeviceSurfacePresentModesKHR(&get(), &surface.get(), &count, modes.data());
        if (res2 != VK_SUCCESS) return make_unexpected(res2);

        return modes |
               std::views::transform([](VkPresentModeKHR& mode) noexcept -> present_mode_khr {
                   return static_cast<present_mode_khr>(mode);
               }) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto create_device(const queue_family& family
    ) noexcept -> std::expected<device, result> {
        {
            auto           device_queue   = VkDeviceQueueCreateInfo{};
            constexpr auto priority       = 1.f;
            device_queue.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            device_queue.queueFamilyIndex = family.index();

            device_queue.queueCount = 1;  // キューファミリーから生成するキューの数
            // 優先順位(2つ作る場合はstd::array{1.f, 0.5f}.data()などを期待)
            device_queue.pQueuePriorities = &priority;

            auto dynamic_rendering  = VkPhysicalDeviceDynamicRenderingFeatures{};
            dynamic_rendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
            dynamic_rendering.dynamicRendering = VK_TRUE;  // 使用する

            auto features =
                VkPhysicalDeviceFeatures{};  // 物理デバイス機能(ゼロ初期化では何も指定しない)

            auto info  = VkDeviceCreateInfo{};
            info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            info.pNext = &dynamic_rendering;  // 動的レンダリング拡張の仕様
            info.queueCreateInfoCount  = 1;   // 実質キューファミリーの数
            info.pQueueCreateInfos     = &device_queue;
            info.pEnabledFeatures      = &features;
            info.enabledExtensionCount = 1;

            // 拡張機能名
            constexpr auto* extension    = "VK_KHR_swapchain";
            info.ppEnabledExtensionNames = &extension;

            auto*      raw = static_cast<raw::device*>(nullptr);
            const auto res = vkCreateDevice(&get(), &info, nullptr, &raw);
            if (res != VK_SUCCESS) return make_unexpected(res);

            return device{*raw};
        }
    }

    [[nodiscard]] auto get() noexcept -> raw::physical_device& { return raw_; }

    [[nodiscard]] auto get() const noexcept -> raw::physical_device& { return raw_; }

  private:
    explicit physical_device(raw::physical_device& raw) noexcept : raw_{std::move(raw)} {}

    [[nodiscard]] auto get_property() const noexcept -> VkPhysicalDeviceProperties {
        auto property = VkPhysicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(&get(), &property);
        return property;
    }

    raii::physical_device raw_;
};
}  // namespace hgn