#pragma once

#include <vulkan/vulkan.h>
#include <expected>
#include <functional>
#include <vector>

#include "hgn/device.hpp"
#include "hgn/swapchain.hpp"
#include "hgn/types.hpp"

namespace hgn {
using image = VkImage_T;

[[nodiscard]] inline auto try_get_swapchain_images_khr(device& dev, swapchain& chain) noexcept
    -> std::expected<std::vector<std::reference_wrapper<image>>, result> {
    auto       count = u32{};
    const auto res1  = vkGetSwapchainImagesKHR(&dev, &chain, &count, nullptr);
    if (res1 != VK_SUCCESS) return make_result(res1);

    auto images = std::vector<image*>(count);

    const auto res2 = vkGetSwapchainImagesKHR(&dev, &chain, &count, images.data());
    if (res2 != VK_SUCCESS) return make_result(res2);

    auto out = std::vector<std::reference_wrapper<image>>{};
    out.reserve(count);
    out.assign_range(
        images | std::views::transform([](image* ptr) noexcept -> std::reference_wrapper<image> {
            assert(ptr != nullptr);
            return std::ref(*ptr);
        })
    );

    return out;
};
}  // namespace hgn