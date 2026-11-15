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
        auto pipelines = pipeline_container::create(dev, swapchain_format);
        if (not pipelines)
            return akr::make_error(akr::runtime_error, "Failed to create pipelines.");

        auto sampler = dev.create_sampler();
        if (not sampler) return akr::make_error(akr::runtime_error, "Failed to create sampler.");

        return renderer{std::move(*pipelines), std::move(*sampler)};
    }

  private:
    renderer(pipeline_container pipelines, hgn::sampler sampler) noexcept
        : pipelines_{std::move(pipelines)}, sampler_{std::move(sampler)} {}

    pipeline_container pipelines_;
    hgn::sampler       sampler_;
};
}  // namespace sui