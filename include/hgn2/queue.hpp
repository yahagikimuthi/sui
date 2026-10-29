#pragma once

#include <cassert>
#include <cstdint>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn2/device.hpp"

namespace hgn {
class queue final {
    friend class device;

  public:
    queue(const queue&) noexcept                    = delete;
    auto operator=(const queue&) noexcept -> queue& = delete;
    queue(queue&&) noexcept                         = default;
    auto operator=(queue&&) noexcept -> queue&      = default;
    ~queue() noexcept                               = default;

    [[nodiscard]] auto queue_family_index() const noexcept -> std::uint32_t {
        return queue_family_index_;
    }

  private:
    explicit queue(VkQueue_T& raw, const std::uint32_t queue_family_index) noexcept
        : raw_{std::ref(raw)}, queue_family_index_{queue_family_index} {}

    std::reference_wrapper<VkQueue_T> raw_;
    std::uint32_t                     queue_family_index_;
};

inline auto device::get_queue(const std::uint32_t queue_family_index) noexcept -> queue {
    auto* raw = static_cast<VkQueue_T*>(nullptr);
    vkGetDeviceQueue(&get(), queue_family_index, 0, &raw);

    assert(raw != nullptr);
    return queue{*raw, queue_family_index};
}
}  // namespace hgn