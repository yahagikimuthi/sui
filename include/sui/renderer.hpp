#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/image.hpp>

#include "sui/standard.hpp"

namespace sui {
class renderer final {
  public:
    [[nodiscard]] static auto create(hgn::device& dev, const hgn::format swapchain_format) noexcept
        -> std::expected<renderer, akr::error> {
        auto standard = pipeline::create_standards(dev, swapchain_format);
        if (not standard)
            return akr::make_error(
                akr::runtime_error, "Failed to create standard pipeline.", standard.error()
            );

        auto texture = pipeline::create_standards(dev, swapchain_format);
        if (not texture)
            return akr::make_error(
                akr::runtime_error, "Failed to create texture pipeline.", texture.error()
            );

        auto sampler = dev.create_sampler();
        if (not sampler) return akr::make_error(akr::runtime_error, "Failed to create sampler.");

        return renderer{std::move(*standard), std::move(*texture), std::move(*sampler)};
    }

  private:
    renderer(pipeline standard, pipeline texture, hgn::sampler sampler) noexcept
        : standard_{std::move(standard)},
          texture_{std::move(texture)},
          sampler_{std::move(sampler)} {}

    pipeline     standard_;
    pipeline     texture_;
    hgn::sampler sampler_;
};
}  // namespace sui