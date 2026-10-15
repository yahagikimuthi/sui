#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/physical_device.hpp"

namespace hgn {
class device_queue_create_info final {
  public:
    explicit device_queue_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    }

    device_queue_create_info(const device_queue_create_info& other) noexcept
        : native_{other.native_}, priority_{other.priority_} {
        reset();
    }

    auto operator=(const device_queue_create_info& other) noexcept -> device_queue_create_info& {
        if (this == &other) return *this;

        native_   = other.native_;
        priority_ = other.priority_;

        reset();
        return *this;
    }

    device_queue_create_info(device_queue_create_info&& other) noexcept
        : native_{other.native_}, priority_{other.priority_} {
        reset();
    }

    auto operator=(device_queue_create_info&& other) noexcept -> device_queue_create_info& {
        if (this == &other) return *this;

        native_   = other.native_;
        priority_ = other.priority_;

        reset();
        return *this;
    }

    ~device_queue_create_info() noexcept = default;

    auto queue_family_index(const std::uint32_t idx) noexcept -> device_queue_create_info& {
        native_.queueFamilyIndex = idx;
        return *this;
    }

    auto queue_count(const std::uint32_t cnt) noexcept -> device_queue_create_info& {
        native_.queueCount = cnt;
        return *this;
    }

    auto queue_priorities(const float priority) noexcept -> device_queue_create_info& {
        priority_                = priority;
        native_.pQueuePriorities = &priority_;
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkDeviceQueueCreateInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pQueuePriorities != nullptr) native_.pQueuePriorities = &priority_;
    }

    VkDeviceQueueCreateInfo native_{};
    float                   priority_{};
};

class physical_device_features final {
  public:
    [[nodiscard]] auto native() const noexcept -> const VkPhysicalDeviceFeatures& {
        return native_;
    }

  private:
    VkPhysicalDeviceFeatures native_{};
};

class physical_device_dynamic_rendering_features final {
  public:
    explicit physical_device_dynamic_rendering_features() noexcept {
        native_.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        native_.dynamicRendering = VK_TRUE;
    }

    [[nodiscard]] auto native() const noexcept -> const VkPhysicalDeviceDynamicRenderingFeatures& {
        return native_;
    }

  private:
    VkPhysicalDeviceDynamicRenderingFeatures native_{};
};

class device_create_info final {
  public:
    explicit device_create_info() noexcept { native_.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; }

    device_create_info(const device_create_info& other) noexcept
        : native_{other.native_}, features_{other.features_}, infos_{other.infos_} {
        reset();
    }

    auto operator=(const device_create_info& other) noexcept -> device_create_info& {
        if (this == &other) return *this;
        native_   = other.native_;
        features_ = other.features_;
        infos_    = other.infos_;

        reset();
        return *this;
    }

    device_create_info(device_create_info&& other) noexcept
        : native_{other.native_}, features_{other.features_}, infos_{std::move(other.infos_)} {
        reset();
    }

    auto operator=(device_create_info&& other) noexcept -> device_create_info& {
        if (this == &other) return *this;

        native_   = other.native_;
        features_ = other.features_;
        infos_    = other.infos_;

        reset();
        return *this;
    }

    ~device_create_info() noexcept = default;

    auto dynamic_rendering_features(
        const physical_device_dynamic_rendering_features& features
    ) noexcept -> device_create_info& {
        dynamic_features_ = features.native();
        native_.pNext     = &dynamic_features_;
        return *this;
    }

    auto queue_create_infos(const std::span<const device_queue_create_info> infos) noexcept
        -> device_create_info& {
        infos_ = std::vector<VkDeviceQueueCreateInfo>{};
        infos_.reserve(infos.size());
        infos_.assign_range(
            infos | std::views::transform(
                        [](const device_queue_create_info& info) noexcept
                            -> const VkDeviceQueueCreateInfo& { return info.native(); }
                    )
        );

        native_.queueCreateInfoCount = static_cast<std::uint32_t>(infos_.size());
        native_.pQueueCreateInfos    = infos_.data();
        return *this;
    }

    auto features(const physical_device_features& features) noexcept -> device_create_info& {
        features_                = features.native();
        native_.pEnabledFeatures = &features_;
        return *this;
    }

    auto extensions(std::span<const std::string_view> vec) noexcept -> device_create_info& {
        extensions_.reserve(vec.size());
        extensions_.assign_range(
            vec | std::views::transform([](const std::string_view view) noexcept -> const char* {
                return view.data();
            })
        );
        native_.enabledExtensionCount   = static_cast<std::uint32_t>(extensions_.size());
        native_.ppEnabledExtensionNames = extensions_.data();
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkDeviceCreateInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pEnabledFeatures != nullptr) native_.pEnabledFeatures = &features_;
        if (native_.pQueueCreateInfos != nullptr) native_.pQueueCreateInfos = infos_.data();
        if (native_.ppEnabledExtensionNames != nullptr)
            native_.ppEnabledExtensionNames = extensions_.data();
        if (native_.pNext != nullptr) native_.pNext = &dynamic_features_;
    }

    VkDeviceCreateInfo                       native_{};
    VkPhysicalDeviceFeatures                 features_{};
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_features_{};
    std::vector<VkDeviceQueueCreateInfo>     infos_;
    std::vector<const char*>                 extensions_;
};

using device = VkDevice_T;

[[nodiscard]] inline auto try_make_device(
    physical_device& physical, const device_create_info& info
) noexcept -> std::expected<std::reference_wrapper<device>, result> {
    auto*      dev = static_cast<device*>(nullptr);
    const auto res = vkCreateDevice(&physical, &info.native(), nullptr, &dev);
    if (res != VK_SUCCESS) return make_result(res);

    assert(dev != nullptr);
    return std::ref(*dev);
}

inline void destroy_device(device& dev) noexcept {
    vkDestroyDevice(&dev, nullptr);
}

using queue = VkQueue_T;

[[nodiscard]] inline auto try_get_device_queue(
    device& dev, const std::uint32_t graphic_family
) noexcept -> std::optional<queue&> {
    auto* out = static_cast<queue*>(nullptr);
    vkGetDeviceQueue(&dev, graphic_family, 0, &out);
    if (out == nullptr) return std::nullopt;
    return *out;
}
}  // namespace hgn