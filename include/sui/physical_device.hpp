#pragma once

#include <cassert>
#include <hgn/device.hpp>
#include <hgn/types.hpp>
#include <optional>
#include <ranges>

#include "sui/types.hpp"

namespace sui {
class physical_device final {
  public:
    explicit physical_device(hgn::physical_device device) noexcept : native_{device} {}

    [[nodiscard]] auto name() const noexcept -> std::string {
        auto properties = hgn::get_physical_device_properties(native_);
        return hgn::physical_device_properties_view{properties}.device_name();
    }

    [[nodiscard]] auto find_queue_families() const noexcept -> std::optional<u32> {
        auto indices        = std::optional<u32>{};
        auto queue_families = hgn::get_physical_device_queue_family_properties(native_);

        for (const auto& [i, queue_family] : std::views::enumerate(queue_families)) {
            if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                indices = static_cast<u32>(i);
                break;
            }
        }

        return indices;
    }

  private:
    hgn::physical_device native_{hgn::null_handle};
};
}  // namespace sui