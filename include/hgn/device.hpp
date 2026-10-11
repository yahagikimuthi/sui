#pragma once

#include <vulkan/vulkan.h>
#include <cassert>
#include <expected>
#include <functional>
#include <optional>
#include <ranges>

#include "hgn/enums.hpp"
#include "hgn/physical_device.hpp"
#include "hgn/types.hpp"

namespace hgn {
class device_queue_create_info final {
  public:
    explicit device_queue_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    }

    auto queue_family_index(const u32 idx) noexcept -> device_queue_create_info& {
        native_.queueFamilyIndex = idx;
        return *this;
    }

    auto queue_count(const u32 cnt) noexcept -> device_queue_create_info& {
        native_.queueCount = cnt;
        return *this;
    }

    auto queue_priorities(const f32 priority) noexcept -> device_queue_create_info& {
        priority_                = priority;
        native_.pQueuePriorities = &priority_;
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkDeviceQueueCreateInfo& { return native_; }

  private:
    VkDeviceQueueCreateInfo native_{};
    f32                     priority_{};
};

class physical_device_features final {
  public:
    [[nodiscard]] auto native() const noexcept -> const VkPhysicalDeviceFeatures& {
        return native_;
    }

  private:
    VkPhysicalDeviceFeatures native_{};
};

class device_create_info final {
  public:
    explicit device_create_info() noexcept { native_.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; }

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

        native_.queueCreateInfoCount = static_cast<u32>(infos_.size());
        native_.pQueueCreateInfos    = infos_.data();
        return *this;
    }

    auto features(const physical_device_features& features) noexcept -> device_create_info& {
        features_                = features.native();
        native_.pEnabledFeatures = &features_;
        return *this;
    }

    auto extensions(std::span<const char* const> vec) noexcept -> device_create_info& {
        native_.enabledExtensionCount   = static_cast<u32>(vec.size());
        native_.ppEnabledExtensionNames = vec.data();
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkDeviceCreateInfo& { return native_; }

  private:
    VkDeviceCreateInfo                   native_{};
    VkPhysicalDeviceFeatures             features_{};
    std::vector<VkDeviceQueueCreateInfo> infos_;
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

[[nodiscard]] inline auto try_get_device_queue(device& dev, const u32 graphic_family) noexcept
    -> std::optional<queue&> {
    auto* out = static_cast<queue*>(nullptr);
    vkGetDeviceQueue(&dev, graphic_family, 0, &out);
    if (out == nullptr) return std::nullopt;
    return *out;
}
}  // namespace hgn