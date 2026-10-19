#pragma once

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

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

    [[nodiscard]] auto native() const noexcept -> const VkShaderModuleCreateInfo& {
        return native_;
    }

  private:
    void reset() noexcept {
        if (native_.pCode != nullptr)
            native_.pCode = reinterpret_cast<const std::uint32_t*>(code_.data());
    }

    VkShaderModuleCreateInfo native_{};
    std::vector<char>        code_;
};
}  // namespace hgn