#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/shader.hpp"

namespace hgn {
class vertex_input_binding_description final {
  public:
    explicit vertex_input_binding_description() noexcept = default;

    auto binding(const std::uint32_t num) noexcept -> vertex_input_binding_description& {
        native_.binding = num;
        return *this;
    }

    auto stride(const std::uint32_t str) noexcept -> vertex_input_binding_description& {
        native_.stride = str;
        return *this;
    }

    auto input_rate(const vertex_input_rate rate) noexcept -> vertex_input_binding_description& {
        native_.inputRate = static_cast<VkVertexInputRate>(rate);
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkVertexInputBindingDescription& {
        return native_;
    }

  private:
    VkVertexInputBindingDescription native_{};
};

class vertex_input_attribute_description final {
  public:
    explicit vertex_input_attribute_description() noexcept = default;

    auto binding(const std::uint32_t num) noexcept -> vertex_input_attribute_description& {
        native_.binding = num;
        return *this;
    }

    auto location(const std::uint32_t num) noexcept -> vertex_input_attribute_description& {
        native_.location = num;
        return *this;
    }

    auto format(const format f) noexcept -> vertex_input_attribute_description& {
        native_.format = static_cast<VkFormat>(f);
        return *this;
    }

    auto offset(const std::uint32_t num) noexcept -> vertex_input_attribute_description& {
        native_.offset = num;
        return *this;
    }

  private:
    VkVertexInputAttributeDescription native_{};
};

class pipeline_vertex_input_state_create_info final {
  public:
    explicit pipeline_vertex_input_state_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    }

    pipeline_vertex_input_state_create_info(
        const pipeline_vertex_input_state_create_info& other
    ) noexcept
        : native_{other.native_}, attributes_{other.attributes_}, binding_{other.binding_} {
        reset();
    }

    auto operator=(const pipeline_vertex_input_state_create_info& other) noexcept
        -> pipeline_vertex_input_state_create_info& {
        if (this == &other) return *this;

        native_     = other.native_;
        attributes_ = other.attributes_;
        binding_    = other.binding_;

        reset();
        return *this;
    }

    pipeline_vertex_input_state_create_info(
        pipeline_vertex_input_state_create_info&& other
    ) noexcept
        : native_{other.native_},
          attributes_{std::move(other.attributes_)},
          binding_{other.binding_} {
        reset();
    }

    auto operator=(pipeline_vertex_input_state_create_info&& other) noexcept
        -> pipeline_vertex_input_state_create_info& {
        if (this == &other) return *this;

        native_     = other.native_;
        attributes_ = std::move(other.attributes_);
        binding_    = other.binding_;

        reset();
        return *this;
    }

    ~pipeline_vertex_input_state_create_info() noexcept = default;

    auto vertex_attribute_descriptions(
        const std::span<vertex_input_attribute_description> descriptions
    ) noexcept -> pipeline_vertex_input_state_create_info& {
        attributes_.reserve(descriptions.size());
        attributes_.assign_range(
            descriptions |
            std::views::transform(
                [](const vertex_input_attribute_description& description) noexcept
                    -> VkVertexInputAttributeDescription {
                    return std::bit_cast<VkVertexInputAttributeDescription>(description);
                }
            )
        );

        native_.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributes_.size());
        native_.pVertexAttributeDescriptions    = attributes_.data();
        return *this;
    }

    auto vertex_binding_description(const vertex_input_binding_description& description) noexcept
        -> pipeline_vertex_input_state_create_info& {
        binding_ = std::bit_cast<VkVertexInputBindingDescription>(description);
        native_.vertexBindingDescriptionCount = 1;
        native_.pVertexBindingDescriptions    = &binding_;
        return *this;
    }

  private:
    void reset() noexcept {
        if (native_.pVertexAttributeDescriptions != nullptr)
            native_.pVertexAttributeDescriptions = attributes_.data();
        if (native_.pVertexBindingDescriptions != nullptr)
            native_.pVertexBindingDescriptions = &binding_;
    }

    VkPipelineVertexInputStateCreateInfo           native_{};
    std::vector<VkVertexInputAttributeDescription> attributes_;
    VkVertexInputBindingDescription                binding_{};
};

class pipeline_input_assembly_state_create_info final {
  public:
    explicit pipeline_input_assembly_state_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        native_.primitiveRestartEnable = VK_FALSE;
    }

    auto topology(const primitive_topology type) noexcept
        -> pipeline_input_assembly_state_create_info& {
        native_.topology = static_cast<VkPrimitiveTopology>(type);
        return *this;
    }

  private:
    VkPipelineInputAssemblyStateCreateInfo native_{};
};

class pipeline_dynamic_state_create_info final {
  public:
    explicit pipeline_dynamic_state_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    }

    auto dynamic_states(const std::span<const VkDynamicState> states) noexcept
        -> pipeline_dynamic_state_create_info& {
        native_.dynamicStateCount = static_cast<std::uint32_t>(states.size());
        native_.pDynamicStates    = states.data();
        return *this;
    }

  private:
    VkPipelineDynamicStateCreateInfo native_{};
};

class pipeline_viewport_state_create_info final {
  public:
    explicit constexpr pipeline_viewport_state_create_info() noexcept {
        native_.sType         = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        native_.viewportCount = 1;
        native_.scissorCount  = 1;
    }

  private:
    VkPipelineViewportStateCreateInfo native_{};
};

class pipeline_rasterization_state_create_info final {
  public:
    explicit pipeline_rasterization_state_create_info() noexcept {
        native_.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        native_.depthClampEnable = VK_FALSE;
        native_.rasterizerDiscardEnable = VK_FALSE;
        native_.polygonMode             = VK_POLYGON_MODE_FILL;
        native_.lineWidth               = 1.f;
        native_.cullMode                = VK_CULL_MODE_BACK_BIT;
        native_.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        native_.depthBiasEnable         = VK_FALSE;
    }

  private:
    VkPipelineRasterizationStateCreateInfo native_{};
};

class pipeline_multisample_state_create_info final {
  public:
    explicit pipeline_multisample_state_create_info() noexcept {
        native_.sType                = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        native_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    auto is_sample_shadring(const bool cond) noexcept -> pipeline_multisample_state_create_info& {
        native_.sampleShadingEnable = static_cast<VkBool32>(cond);
        return *this;
    }

  private:
    VkPipelineMultisampleStateCreateInfo native_{};  // NOLINT
};

class pipeline_color_blend_attachment_state final {
  public:
    explicit pipeline_color_blend_attachment_state() noexcept {
        native_.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                 VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        native_.blendEnable    = VK_FALSE;

        auto x = std::vector<bool>();
        auto i = x.front();
        i.flip();
    }

    [[nodiscard]] auto native() const noexcept -> const VkPipelineColorBlendAttachmentState& {
        return native_;
    }

  private:
    VkPipelineColorBlendAttachmentState native_{};
};

class pipeline_color_blend_state_create_info final {
  public:
    explicit pipeline_color_blend_state_create_info() noexcept {
        native_.sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        native_.logicOpEnable = VK_FALSE;
    }

    auto attachment(const pipeline_color_blend_attachment_state& state) noexcept
        -> pipeline_color_blend_state_create_info& {
        native_.attachmentCount = 1;
        native_.pAttachments    = &state.native();
        return *this;
    }

  private:
    VkPipelineColorBlendStateCreateInfo native_{};
};

class pipeline_shader_stage_create_info final {
  public:
    explicit pipeline_shader_stage_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        native_.pName = "main";
    }

    auto stage(const shader_stage_flag_bits bit) noexcept -> pipeline_shader_stage_create_info& {
        native_.stage = static_cast<VkShaderStageFlagBits>(bit);
        return *this;
    }

    auto module(shader_module& mod) noexcept -> pipeline_shader_stage_create_info& {
        native_.module = &mod;
        return *this;
    }

  private:
    VkPipelineShaderStageCreateInfo native_{};  // NOLINT
};

using pipeline_layout = VkPipelineLayout_T;

class pipeline_layout_create_info final {
  public:
    explicit pipeline_layout_create_info() noexcept {
        native_.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        native_.setLayoutCount         = 0;
        native_.pushConstantRangeCount = 0;
    }

    [[nodiscard]] auto native() const noexcept -> const VkPipelineLayoutCreateInfo& {
        return native_;
    }

  private:
    VkPipelineLayoutCreateInfo native_{};
};

[[nodiscard]] inline auto try_make_pipeline_layout(
    device& dev, const pipeline_layout_create_info& info
) noexcept -> std::expected<std::reference_wrapper<pipeline_layout>, result> {
    auto*      layout_ptr = static_cast<pipeline_layout*>(nullptr);
    const auto res        = vkCreatePipelineLayout(&dev, &info.native(), nullptr, &layout_ptr);

    if (res != VK_SUCCESS) return make_result(res);

    assert(layout_ptr != nullptr);
    return std::ref(*layout_ptr);
}

class pipeline_rendering_create_info final {
  public:
    explicit pipeline_rendering_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    }

    auto color_attachment_format(const format f) noexcept -> pipeline_rendering_create_info& {
        format_                         = static_cast<VkFormat>(f);
        native_.colorAttachmentCount    = 1;
        native_.pColorAttachmentFormats = &format_;
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkPipelineRenderingCreateInfo& {
        return native_;
    }

  private:
    VkPipelineRenderingCreateInfo native_{};
    VkFormat                      format_{static_cast<VkFormat>(0)};
};

class graphics_pipeline_create_info final {
  public:
    explicit graphics_pipeline_create_info() noexcept {
        native_.sType   = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        native_.subpass = 0;
    }

    auto next(const pipeline_rendering_create_info& info) noexcept
        -> graphics_pipeline_create_info& {
        native_.pNext = &info.native();
        return *this;
    }

    [[nodiscard]] auto native() const noexcept -> const VkGraphicsPipelineCreateInfo& {
        return native_;
    }

  private:
    VkGraphicsPipelineCreateInfo native_{};
};

using pipeline = VkPipeline_T;

[[nodiscard]] inline auto try_make_graphics_pipeline(
    device& dev, const graphics_pipeline_create_info& info
) noexcept -> std::expected<std::reference_wrapper<pipeline>, result> {
    auto* pipe_ptr = static_cast<pipeline*>(nullptr);

    const auto res =
        vkCreateGraphicsPipelines(&dev, nullptr, 1, &info.native(), nullptr, &pipe_ptr);
    if (res != VK_SUCCESS) return make_result(res);

    assert(pipe_ptr != nullptr);
    return std::ref(*pipe_ptr);
}
}  // namespace hgn