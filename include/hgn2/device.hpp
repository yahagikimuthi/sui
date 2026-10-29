#pragma once

#include <cstdint>
#include <functional>

#include <vulkan/vulkan.h>

namespace hgn {
class device_queue final {
  public:
    class builder final {
      public:
        auto queue_family_index(const std::uint32_t index) noexcept -> builder& {
            queue_family_index_ = index;
            return *this;
        }

        [[nodiscard]] auto build() const noexcept -> device_queue {}

      private:
        std::uint32_t queue_family_index_{};
    };

    [[nodiscard]] auto info() const noexcept -> const VkDeviceQueueCreateInfo& { return info_; }

  private:
    VkDeviceQueueCreateInfo info_;
};

class device final {
  public:
    device(const device&) noexcept                    = delete;
    auto operator=(const device&) noexcept -> device& = delete;
    device(device&&) noexcept                         = default;
    auto operator=(device&&) noexcept -> device&      = default;
    ~device() noexcept                                = default;

    class builder final {
      public:
        explicit builder() noexcept = default;

        auto queue_family_index(const std::uint32_t index) noexcept -> builder&;

        [[nodiscard]] auto build() const noexcept -> device;

      private:
        std::uint32_t queue_family_index_{};
    };

  private:
    friend class device::builder;

    explicit device(VkDevice_T& raw) noexcept : raw_{std::ref(raw)} {}

    std::reference_wrapper<VkDevice_T> raw_;
};
}  // namespace hgn

#define HGN_DETAIL_DEVICE_IMPL_HPP_
#include "hgn2/detail/device.hpp"
#undef HGN_DETAIL_DEVICE_IMPL_HPP_