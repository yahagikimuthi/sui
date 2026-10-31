#pragma once

#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raii/rendering.hpp"
#include "hgn/raw.hpp"

namespace hgn {
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
}  // namespace hgn