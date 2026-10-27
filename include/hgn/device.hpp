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

#include "hgn/declaration.hpp"
#include "hgn/enums.hpp"

namespace hgn {
class device_queue_create_info final {
  public:
    explicit device_queue_create_info() noexcept {
        native_.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        native_.queueCount       = 1;
        native_.pQueuePriorities = &priority_;
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

    [[nodiscard]] auto get() const noexcept -> const VkDeviceQueueCreateInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pQueuePriorities != nullptr) native_.pQueuePriorities = &priority_;
    }

    VkDeviceQueueCreateInfo native_{};
    float                   priority_{1};
};

class physical_device_features final {
  public:
    explicit physical_device_features() noexcept = default;

    [[nodiscard]] auto get() const noexcept -> const VkPhysicalDeviceFeatures& { return native_; }

  private:
    VkPhysicalDeviceFeatures native_{};
};

class physical_device_dynamic_rendering_features final {
  public:
    explicit physical_device_dynamic_rendering_features() noexcept {
        native_.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        native_.dynamicRendering = VK_TRUE;
    }

    [[nodiscard]] auto get() const noexcept -> const VkPhysicalDeviceDynamicRenderingFeatures& {
        return native_;
    }

  private:
    VkPhysicalDeviceDynamicRenderingFeatures native_{};
};

class device_create_info final {
  public:
    explicit device_create_info() noexcept { native_.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; }

    device_create_info(const device_create_info& other) noexcept
        : native_{other.native_},
          queue_info_{other.queue_info_},
          features_{other.features_},
          dynamic_features_{other.dynamic_features_},
          extensions_{other.extensions_} {
        reset();
    }

    auto operator=(const device_create_info& other) noexcept -> device_create_info& {
        if (this == &other) return *this;

        native_           = other.native_;
        queue_info_       = other.queue_info_;
        features_         = other.features_;
        dynamic_features_ = other.dynamic_features_;
        extensions_       = other.extensions_;

        reset();
        return *this;
    }

    device_create_info(device_create_info&& other) noexcept
        : native_{other.native_},
          queue_info_{other.queue_info_},
          features_{other.features_},
          dynamic_features_{other.dynamic_features_},
          extensions_{std::move(other.extensions_)} {
        reset();
    }

    auto operator=(device_create_info&& other) noexcept -> device_create_info& {
        if (this == &other) return *this;

        native_           = other.native_;
        queue_info_       = other.queue_info_;
        features_         = other.features_;
        dynamic_features_ = other.dynamic_features_;
        extensions_       = std::move(other.extensions_);

        reset();
        return *this;
    }

    ~device_create_info() noexcept = default;

    auto dynamic_rendering_features(
        const physical_device_dynamic_rendering_features& features
    ) noexcept -> device_create_info& {
        dynamic_features_ = features;
        native_.pNext     = &dynamic_features_.get();
        return *this;
    }

    auto queue_create_info(const device_queue_create_info& info) noexcept -> device_create_info& {
        queue_info_                  = info;
        native_.queueCreateInfoCount = 1;
        native_.pQueueCreateInfos    = &queue_info_.get();
        return *this;
    }

    auto features(const physical_device_features& features) noexcept -> device_create_info& {
        features_                = features;
        native_.pEnabledFeatures = &features_.get();
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

    [[nodiscard]] auto get() const noexcept -> const VkDeviceCreateInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pEnabledFeatures != nullptr) native_.pEnabledFeatures = &features_.get();
        if (native_.pQueueCreateInfos != nullptr) native_.pQueueCreateInfos = &queue_info_.get();
        if (native_.ppEnabledExtensionNames != nullptr)
            native_.ppEnabledExtensionNames = extensions_.data();
        if (native_.pNext != nullptr) native_.pNext = &dynamic_features_.get();
    }

    VkDeviceCreateInfo                         native_{};
    device_queue_create_info                   queue_info_;
    physical_device_features                   features_;
    physical_device_dynamic_rendering_features dynamic_features_;
    std::vector<const char*>                   extensions_;
};

[[nodiscard]] inline auto try_make_device(
    physical_device& physical, const device_create_info& info
) noexcept -> std::expected<std::reference_wrapper<device>, result> {
    auto*      dev = static_cast<device*>(nullptr);
    const auto res = vkCreateDevice(&physical, &info.get(), nullptr, &dev);
    if (res != VK_SUCCESS) return make_result(res);

    assert(dev != nullptr);
    return std::ref(*dev);
}

inline void destroy_device(device& dev) noexcept {
    vkDestroyDevice(&dev, nullptr);
}

[[nodiscard]] inline auto try_device_wait_idle(device& dev) noexcept
    -> std::expected<void, result> {
    const auto res = vkDeviceWaitIdle(&dev);
    if (res != VK_SUCCESS) return make_result(res);
    return {};
}

[[nodiscard]] inline auto try_get_device_queue(
    device& dev, const std::uint32_t family_indices
) noexcept -> std::optional<queue&> {
    auto* out = static_cast<queue*>(nullptr);
    vkGetDeviceQueue(&dev, family_indices, 0, &out);
    if (out == nullptr) return std::nullopt;
    return *out;
}
}  // namespace hgn