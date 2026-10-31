#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <limits>
#include <ranges>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/raii/image.hpp"
#include "hgn/raw.hpp"
#include "hgn/sync.hpp"

namespace hgn {
class swapchain_khr final {
    friend class swapchain_khr_builder;

  public:
    swapchain_khr(const swapchain_khr&) noexcept                    = delete;
    auto operator=(const swapchain_khr&) noexcept -> swapchain_khr& = delete;
    swapchain_khr(swapchain_khr&&) noexcept                         = default;
    auto operator=(swapchain_khr&&) noexcept -> swapchain_khr&      = default;
    ~swapchain_khr() noexcept                                       = default;

    [[nodiscard]] auto get_images_khr() noexcept -> std::expected<std::vector<image>, result> {
        auto       count = std::uint32_t{};
        const auto res1  = vkGetSwapchainImagesKHR(&raw_.init_device(), &get(), &count, nullptr);
        if (res1 != VK_SUCCESS) return make_unexpected(res1);

        auto       raws = std::vector<raw::image*>(count);
        const auto res2 = vkGetSwapchainImagesKHR(&raw_.init_device(), &get(), &count, raws.data());
        if (res2 != VK_SUCCESS) return make_unexpected(res2);

        return raws | std::views::transform([](raw::image* raw) noexcept -> image {
                   assert(raw != nullptr);
                   return image{*raw};
               }) |
               std::ranges::to<std::vector>();
    }

    [[nodiscard]] auto acquire_next_image(semaphore& semapho) noexcept
        -> std::expected<std::uint32_t, result> {
        auto       index = std::uint32_t{};
        const auto res   = vkAcquireNextImageKHR(
            &raw_.init_device(),
            &get(),
            std::numeric_limits<std::uint32_t>::max(),
            &semapho.get(),
            nullptr,
            &index
        );
        if (res != VK_SUCCESS) return make_unexpected(res);

        return index;
    }

    [[nodiscard]] auto image_format() const noexcept -> format { return image_format_; }

    [[nodiscard]] auto get() noexcept -> VkSwapchainKHR_T& { return raw_; }

  private:
    explicit swapchain_khr(VkSwapchainKHR_T& raw, raw::device& dev, const format fmt) noexcept
        : raw_{std::move(raw), dev}, image_format_{fmt} {}

    raii::swapchain_khr raw_;
    format              image_format_;
};
}  // namespace hgn