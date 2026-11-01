#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <ranges>
#include <utility>
#include <vector>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/physical.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn::detail {
class instance_creator;
}  // namespace hgn::detail

namespace hgn {
class instance final {
    friend class detail::instance_creator;

  public:
    instance(const instance&) noexcept                    = delete;
    auto operator=(const instance&) noexcept -> instance& = delete;
    instance(instance&&) noexcept                         = default;
    auto operator=(instance&&) noexcept -> instance&      = default;
    ~instance() noexcept                                  = default;

    [[nodiscard]] auto get_physicals() noexcept
        -> std::expected<std::vector<physical_device>, result> {
        auto count = std::uint32_t{};
        if (const auto res = vkEnumeratePhysicalDevices(&get(), &count, nullptr);
            res != VK_SUCCESS) {
            return make_unexpected(res);
        }

        auto devices = std::vector<raw::physical_device*>(count);
        if (const auto res = vkEnumeratePhysicalDevices(&get(), &count, devices.data());
            res != VK_SUCCESS) {
            return make_unexpected(res);
        }

        return devices |
               std::views::transform([](raw::physical_device* ptr) noexcept -> physical_device {
                   assert(ptr != nullptr);
                   return physical_device{*ptr};
               }) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto get() noexcept -> raw::instance& { return raw_; }

  private:
    explicit instance(raw::instance& raw) noexcept : raw_{std::move(raw)} {}

    raii::instance raw_;
};
}  // namespace hgn