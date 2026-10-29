#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

namespace hgn {
class device_queue final {
  public:
    device_queue(const device_queue& other) noexcept
        : info_{other.info_}, property_{other.property_} {
        reset();
    }

    auto operator=(const device_queue& other) noexcept -> device_queue& {
        if (this == &other) return *this;

        info_     = other.info_;
        property_ = other.property_;

        reset();
        return *this;
    }

    device_queue(device_queue&& other) noexcept : info_{other.info_}, property_{other.property_} {
        reset();
    }

    auto operator=(device_queue&& other) noexcept -> device_queue& {
        if (this == &other) return *this;

        info_     = other.info_;
        property_ = other.property_;

        reset();
        return *this;
    }

    ~device_queue() noexcept = default;

    [[nodiscard]] auto info() const noexcept -> const VkDeviceQueueCreateInfo& { return info_; }

    class builder final {
      public:
        auto queue_family_index(const std::uint32_t index) noexcept -> builder& {
            queue_family_index_ = index;
            return *this;
        }

        [[nodiscard]] auto build() const noexcept -> device_queue {
            return device_queue{queue_family_index_};
        }

      private:
        std::uint32_t queue_family_index_{};
    };

  private:
    friend class device_queue::builder;

    explicit device_queue(const std::uint32_t queue_family_index) noexcept {
        info_.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info_.queueCount       = 1;
        info_.pQueuePriorities = &property_;
        info_.queueFamilyIndex = queue_family_index;
    }

    void reset() noexcept {
        if (info_.pQueuePriorities != nullptr) info_.pQueuePriorities = &property_;
    }

    VkDeviceQueueCreateInfo info_{};
    float                   property_{1};
};
}  // namespace hgn