#pragma once

#include <functional>

#include <vulkan/vulkan.h>

namespace hgn {
class physical_device final {
    friend class instance;

  public:
    physical_device(const physical_device&) noexcept                    = delete;
    auto operator=(const physical_device&) noexcept -> physical_device& = delete;
    physical_device(physical_device&&) noexcept                         = default;
    auto operator=(physical_device&&) noexcept -> physical_device&      = default;
    ~physical_device() noexcept                                         = default;

    [[nodiscard]] auto get() noexcept -> VkPhysicalDevice_T& { return raw_; }

  private:
    explicit physical_device(VkPhysicalDevice_T& raw) noexcept : raw_{std::ref(raw)} {}

    std::reference_wrapper<VkPhysicalDevice_T> raw_;
};
}  // namespace hgn

#define HGN_DETAIL_PHYSICAL_IMPL_HPP_
#include "hgn2/detail/physical.impl.hpp"
#undef HGN_DETAIL_PHYSICAL_IMPL_HPP_