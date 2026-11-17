#pragma once

#include <expected>
#include <utility>

#include <akr/akr.hpp>
#include <hgn/allocator.hpp>
#include <hgn/device.hpp>
#include <hgn/enums.hpp>
#include <hgn/image.hpp>

#include "sui/buffer.hpp"
#include "sui/standard.hpp"

namespace sui {
class renderer final {
  public:
    [[nodiscard]] static auto create(
        hgn::allocator& allocator, hgn::device& dev, const hgn::format swapchain_format
    ) noexcept -> std::expected<renderer, akr::error> {
        auto pipelines = pipeline_container::create(dev, swapchain_format);
        if (not pipelines)
            return pipelines.error().rethrow_runtime("Failed to create pipeline object.");

        auto sampler = dev.create_sampler();
        if (not sampler) return akr::throw_runtime("Failed to create sampler.");

        auto buffers = buffer_container::create(allocator);
        if (not buffers) return buffers.error().rethrow_runtime("Failed to create buffers.");

        return renderer{std::move(*pipelines), std::move(*buffers), std::move(*sampler)};
    }

  private:
    renderer(pipeline_container pipelines, buffer_container buffers, hgn::sampler sampler) noexcept
        : pipelines_{std::move(pipelines)},
          buffers_{std::move(buffers)},
          sampler_{std::move(sampler)} {}

    pipeline_container pipelines_;
    buffer_container   buffers_;
    hgn::sampler       sampler_;
};
}  // namespace sui