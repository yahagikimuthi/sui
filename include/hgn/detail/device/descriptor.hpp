#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/descriptor.hpp"
#include "hgn/enums.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class descriptor_set_layout_creator;
class descriptor_set_layout_binding_pusher final {
  public:
    explicit descriptor_set_layout_binding_pusher(
        descriptor_set_layout_creator& creator, std::vector<VkDescriptorSetLayoutBinding>& vec
    )
        : creator_{std::ref(creator)}, vec_{std::ref(vec)} {
        binding_.descriptorCount = 1;
    }

    auto binding(const std::uint32_t i) noexcept -> descriptor_set_layout_binding_pusher& {
        binding_.binding = i;
        return *this;
    }

    auto type(const descriptor_type type) noexcept -> descriptor_set_layout_binding_pusher& {
        binding_.descriptorType = static_cast<VkDescriptorType>(type);
        return *this;
    }

    auto shader_stage(const shader_stage_flag_bits bit
    ) noexcept -> descriptor_set_layout_binding_pusher& {
        binding_.stageFlags = static_cast<VkShaderStageFlagBits>(bit);
        return *this;
    }

    auto push() noexcept -> descriptor_set_layout_creator& {
        auto& vec = vec_.get();
        vec.emplace_back(binding_);
        return creator_;
    }

  private:
    VkDescriptorSetLayoutBinding                                      binding_{};
    std::reference_wrapper<descriptor_set_layout_creator>             creator_;
    std::reference_wrapper<std::vector<VkDescriptorSetLayoutBinding>> vec_;
};

class descriptor_set_layout_creator final {
  public:
    explicit descriptor_set_layout_creator(raw::device& dev) noexcept : dev_{std::ref(dev)} {
        info_.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    }

    [[nodiscard]] auto get_binding_pusher() noexcept -> descriptor_set_layout_binding_pusher {
        return descriptor_set_layout_binding_pusher{*this, bindings_};
    }

    [[nodiscard]] auto create() noexcept -> std::expected<descriptor_set_layout, result> {
        info_.bindingCount = static_cast<std::uint32_t>(bindings_.size());
        info_.pBindings    = bindings_.data();

        auto* raw = static_cast<raw::descriptor_set_layout*>(nullptr);
        return make_expected(vkCreateDescriptorSetLayout(&dev_.get(), &info_, nullptr, &raw))
            .transform([&]() noexcept -> descriptor_set_layout {
                return descriptor_set_layout{*raw, dev_.get()};
            });
    }

  private:
    VkDescriptorSetLayoutCreateInfo           info_{};
    std::vector<VkDescriptorSetLayoutBinding> bindings_;
    std::reference_wrapper<raw::device>       dev_;
};
}  // namespace hgn