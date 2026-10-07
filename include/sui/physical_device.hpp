#pragma once

#include <cassert>
#include <expected>
#include <hgn/device.hpp>
#include <hgn/types.hpp>
#include <optional>
#include <ranges>

#include "sui/error.hpp"
#include "sui/types.hpp"

namespace sui {
class physical_device final {
  public:
    explicit physical_device(hgn::physical_device& device) noexcept : native_{device} {}

    [[nodiscard]] auto name() const noexcept -> std::string {
        if (not native_) return std::string{};
        auto properties = hgn::get_physical_device_properties(*native_);
        return hgn::physical_device_properties_view{properties}.device_name();
    }

    [[nodiscard]] auto find_queue_families() const noexcept -> std::optional<u32> {
        if (not native_) return std::nullopt;

        auto indices        = std::optional<u32>{};
        auto queue_families = hgn::get_physical_device_queue_family_properties(*native_);

        for (const auto& [i, queue_family] : std::views::enumerate(queue_families)) {
            if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                indices = static_cast<u32>(i);
                break;
            }
        }

        return indices;
    }

    [[nodiscard]] auto try_make_device(const hgn::device_create_info& info) noexcept
        -> std::expected<hgn::device, error> {
        if (not native_) return make_error(logic_error, "Invalid physical device.");

        auto device = hgn::try_make_device(*native_, info);
        if (not device) return make_error(runtime_error, "Failed to create device.");

        return *device;
    }

  private:
    std::optional<hgn::physical_device&> native_{std::nullopt};
};
}  // namespace sui