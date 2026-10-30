#pragma once

#include <utility>

#include <vulkan/vulkan.h>

#include "hgn2/raii/rendering.hpp"

namespace hgn {
class shader_module final {
    friend class device;

  public:
    shader_module(const shader_module&) noexcept                    = delete;
    auto operator=(const shader_module&) noexcept -> shader_module& = delete;
    shader_module(shader_module&&) noexcept                         = default;
    auto operator=(shader_module&&) noexcept -> shader_module&      = default;
    ~shader_module() noexcept                                       = default;

    [[nodiscard]] auto get() noexcept -> VkShaderModule_T& { return raw_; }

  private:
    explicit shader_module(VkShaderModule_T& raw, VkDevice_T& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::shader_module raw_;
};
}  // namespace hgn