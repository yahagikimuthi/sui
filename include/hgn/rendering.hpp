#pragma once

#include <utility>

#include "hgn/raii/rendering.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class pipeline_layout final {
    friend class device;

  public:
    [[nodiscard]] auto get() noexcept -> raw::pipeline_layout& { return raw_; }

  private:
    explicit pipeline_layout(raw::pipeline_layout& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::pipeline_layout raw_;
};

class shader_module final {
    friend class device;

  public:
    shader_module(const shader_module&) noexcept                    = delete;
    auto operator=(const shader_module&) noexcept -> shader_module& = delete;
    shader_module(shader_module&&) noexcept                         = default;
    auto operator=(shader_module&&) noexcept -> shader_module&      = default;
    ~shader_module() noexcept                                       = default;

    [[nodiscard]] auto get() noexcept -> raw::shader_module& { return raw_; }

  private:
    explicit shader_module(raw::shader_module& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::shader_module raw_;
};

class graphics_pipeline final {
    friend class graphics_pipeline_creator;

  public:
    [[nodiscard]] auto get() noexcept -> raw::pipeline& { return raw_; }

  private:
    explicit graphics_pipeline(raw::pipeline& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::pipeline raw_;
};
}  // namespace hgn