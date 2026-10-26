#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <span>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"

namespace hgn {
using shader_module = VkShaderModule_T;

class shader_module_create_info final {
  public:
    explicit shader_module_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    }

    shader_module_create_info(const shader_module_create_info& other) noexcept
        : native_{other.native_}, code_{other.code_} {
        reset();
    }

    auto operator=(const shader_module_create_info& other) noexcept -> shader_module_create_info& {
        if (this == &other) return *this;

        native_ = other.native_;
        code_   = other.code_;

        reset();
        return *this;
    }

    shader_module_create_info(shader_module_create_info&& other) noexcept
        : native_{other.native_}, code_{std::move(other.code_)} {
        reset();
    }

    auto operator=(shader_module_create_info&& other) noexcept -> shader_module_create_info& {
        if (this == &other) return *this;

        native_ = other.native_;
        code_   = std::move(other.code_);

        reset();
        return *this;
    }

    ~shader_module_create_info() noexcept = default;

    auto code(const std::span<const char> codes) noexcept -> shader_module_create_info& {
        code_.reserve(codes.size());
        code_.assign_range(codes);

        native_.codeSize = code_.size();

        // FIXME No Use reinterpret_cast
        native_.pCode = reinterpret_cast<const std::uint32_t*>(code_.data());

        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkShaderModuleCreateInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pCode != nullptr)
            native_.pCode = reinterpret_cast<const std::uint32_t*>(code_.data());
    }

    VkShaderModuleCreateInfo native_{};
    std::vector<char>        code_;
};

[[nodiscard]] inline auto try_make_shader_module(
    device& dev, const shader_module_create_info& info
) noexcept -> std::expected<std::reference_wrapper<shader_module>, result> {
    auto* shader_ptr = static_cast<shader_module*>(nullptr);

    const auto res = vkCreateShaderModule(&dev, &info.get(), nullptr, &shader_ptr);
    if (res != VK_SUCCESS) return make_result(res);

    assert(shader_ptr != nullptr);
    return std::ref(*shader_ptr);
}

inline void destroy_shader_module(device& dev, shader_module& target) noexcept {
    vkDestroyShaderModule(&dev, &target, nullptr);
}
}  // namespace hgn