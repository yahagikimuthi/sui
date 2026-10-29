#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn2/device_queue.hpp"
#include "hgn2/enums.hpp"
#include "hgn2/physical.hpp"
#include "hgn2/structs.hpp"
#include "hgn2/sync.hpp"

namespace hgn {
class queue;
class device final {
  public:
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

    [[nodiscard]] auto get_queue(const std::uint32_t queue_family_index) noexcept -> queue;

    [[nodiscard]] auto make_semaphore() noexcept -> std::expected<semaphore, result> {
        auto info      = VkSemaphoreCreateInfo{};
        info.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        auto*      raw = static_cast<VkSemaphore_T*>(nullptr);
        const auto res = vkCreateSemaphore(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return semaphore{*raw, get()};
    }

    [[nodiscard]] auto make_fence() noexcept -> std::expected<fence, result> {
        auto info      = VkFenceCreateInfo{};
        info.sType     = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags     = VK_FENCE_CREATE_SIGNALED_BIT;
        auto*      raw = static_cast<VkFence_T*>(nullptr);
        const auto res = vkCreateFence(&get(), &info, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return fence{*raw, get()};
    }

    [[nodiscard]] auto get() noexcept -> VkDevice_T& { return *raw_; }

    class builder final {
      public:
        explicit builder() noexcept = default;

        auto next(const physical_device_dynamic_rendering_features& next) noexcept -> builder& {
            dynamic_rendering_ = next;
            return *this;
        }

        auto queue(const device_queue& dev_queue) noexcept -> builder& {
            queue_ = dev_queue;
            return *this;
        }

        auto features(const physical_device_features& feature) noexcept -> builder& {
            feature_ = feature;
            return *this;
        }

        [[nodiscard]] auto build(physical_device& physical) const noexcept
            -> std::expected<device, result> {
            auto info  = VkDeviceCreateInfo{};
            info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            if (dynamic_rendering_) info.pNext = &dynamic_rendering_->get();
            if (queue_) {
                info.queueCreateInfoCount = 1;
                info.pQueueCreateInfos    = &queue_->info();
            }
            if (feature_) {
                info.pEnabledFeatures = &feature_->get();
            }
            info.enabledExtensionCount   = 1;
            constexpr auto* extension    = "VK_KHR_swapchain";
            info.ppEnabledExtensionNames = &extension;

            auto*      raw = static_cast<VkDevice_T*>(nullptr);
            const auto res = vkCreateDevice(&physical.get(), &info, nullptr, &raw);
            if (res != VK_SUCCESS) return make_unexpected(res);

            assert(raw != nullptr);
            return device{*raw};
        }

      private:
        std::optional<device_queue>                               queue_;
        std::optional<physical_device_features>                   feature_;
        std::optional<physical_device_dynamic_rendering_features> dynamic_rendering_;
    };

  private:
    friend class device::builder;

    explicit device(VkDevice_T& raw) noexcept : raw_{raw} {}

    void destroy() noexcept {
        if (raw_) vkDestroyDevice(&get(), nullptr);
    }

    std::optional<VkDevice_T&> raw_;
};
}  // namespace hgn