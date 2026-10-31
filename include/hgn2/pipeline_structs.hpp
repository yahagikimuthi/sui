#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn2/enums.hpp"
#include "hgn2/pipeline.hpp"
#include "hgn2/pipeline_layout.hpp"
#include "hgn2/shader.hpp"

namespace hgn {
class vertex_input_binding_description final {
  public:
    explicit vertex_input_binding_description(const std::uint32_t stride) noexcept {
        raw_.binding   = 0;
        raw_.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        raw_.stride    = stride;
    }

    [[nodiscard]] auto get() const noexcept -> const VkVertexInputBindingDescription& {
        return raw_;
    }

  private:
    VkVertexInputBindingDescription raw_{};
};

class pipeline_vertex_input_state final {
  public:
    explicit pipeline_vertex_input_state(const VkPipelineVertexInputStateCreateInfo info) noexcept
        : info_{info} {}

    [[nodiscard]] auto info() const noexcept -> const VkPipelineVertexInputStateCreateInfo& {
        return info_;
    }

    class builder final {
      public:
        explicit builder() noexcept {
            binding_.binding   = 0;
            binding_.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            attributes_[0].binding  = 0;
            attributes_[0].location = 0;
            attributes_[0].format   = VK_FORMAT_R32G32_SFLOAT;

            attributes_[1].binding  = 0;
            attributes_[1].location = 1;
            attributes_[1].format   = VK_FORMAT_R32G32_SFLOAT;
        }

        auto binding(const std::uint32_t stride) noexcept -> builder& {
            binding_.stride = stride;
            return *this;
        }

        auto offsets(const std::uint32_t pos1, const std::uint32_t pos2) noexcept -> builder& {
            attributes_[0].offset = pos1;
            attributes_[1].offset = pos2;
            return *this;
        }

        [[nodiscard]] auto build() const noexcept -> pipeline_vertex_input_state {
            auto info  = VkPipelineVertexInputStateCreateInfo{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            info.vertexBindingDescriptionCount   = 1;
            info.pVertexBindingDescriptions      = &binding_;
            info.vertexAttributeDescriptionCount = 2;
            info.pVertexAttributeDescriptions    = attributes_.data();
            return pipeline_vertex_input_state{info};
        }

      private:
        VkVertexInputBindingDescription                  binding_{};
        std::array<VkVertexInputAttributeDescription, 2> attributes_{};
    };

  private:
    VkPipelineVertexInputStateCreateInfo info_{};
};

class pipeline_dynamic_state final {
  public:
    explicit pipeline_dynamic_state(const dynamic_state pos1, const dynamic_state pos2) noexcept {
        info_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

        states_ = {static_cast<VkDynamicState>(pos1), static_cast<VkDynamicState>(pos2)};
        info_.dynamicStateCount = 2;
        info_.pDynamicStates    = states_.data();
    }

    [[nodiscard]] auto get() const noexcept -> const VkPipelineDynamicStateCreateInfo& {
        return info_;
    }

  private:
    VkPipelineDynamicStateCreateInfo info_{};
    std::array<VkDynamicState, 2>    states_{};
};

class graphics_pipeline_builder final {
    friend class device;

  public:
    auto vertex_input_state(const pipeline_vertex_input_state& state) noexcept
        -> graphics_pipeline_builder& {
        input_state_ = state.info();
        return *this;
    }

    auto dynamic_states(const dynamic_state state1, const dynamic_state state2) noexcept
        -> graphics_pipeline_builder& {
        dynamic_states_ = {
            static_cast<VkDynamicState>(state1), static_cast<VkDynamicState>(state2)
        };
        dynamic_state_.dynamicStateCount = 2;
        dynamic_state_.pDynamicStates    = dynamic_states_.data();
        return *this;
    }

    auto color_attachment(const format fmt) noexcept -> graphics_pipeline_builder& {
        color_attachment_                  = static_cast<VkFormat>(fmt);
        rendering_.colorAttachmentCount    = 1;
        rendering_.pColorAttachmentFormats = &color_attachment_;
        return *this;
    }

    auto shaders(shader_module& vert, shader_module& frag) noexcept -> graphics_pipeline_builder& {
        shaders_[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
        shaders_[0].module = &vert.get();

        shaders_[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaders_[1].module = &frag.get();
        return *this;
    }

    auto layout(pipeline_layout& layout) noexcept -> graphics_pipeline_builder& {
        info_.layout = &layout.get();
        return *this;
    }

    [[nodiscard]] auto build() noexcept -> std::expected<graphics_pipeline, result> {
        info_.pVertexInputState   = &input_state_;
        info_.pInputAssemblyState = &assembly_;
        info_.pDynamicState       = &dynamic_state_;
        info_.pViewportState      = &viewport_;
        info_.pRasterizationState = &rasterization_;
        info_.pMultisampleState   = &multisample_;
        info_.pColorBlendState    = &color_blend_;
        info_.stageCount          = 2;
        info_.pStages             = shaders_.data();
        info_.pNext               = &rendering_;

        auto*      raw = static_cast<VkPipeline_T*>(nullptr);
        const auto res = vkCreateGraphicsPipelines(&dev_.get(), nullptr, 1, &info_, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return graphics_pipeline{*raw, dev_.get()};
    }

  private:
    explicit graphics_pipeline_builder(VkDevice_T& dev) noexcept : dev_{std::ref(dev)} {
        info_.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info_.renderPass = nullptr;
        info_.subpass    = 0;

        assembly_.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        assembly_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        assembly_.primitiveRestartEnable = VK_FALSE;

        dynamic_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

        viewport_.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_.viewportCount = 1;
        viewport_.scissorCount  = 1;

        rasterization_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_.depthClampEnable        = VK_FALSE;
        rasterization_.rasterizerDiscardEnable = VK_FALSE;
        rasterization_.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterization_.lineWidth               = 1.f;
        rasterization_.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterization_.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        rasterization_.depthBiasEnable         = VK_FALSE;

        multisample_.sType               = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_.sampleShadingEnable = VK_FALSE;
        multisample_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        color_blend_attachment_.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_.blendEnable = VK_FALSE;

        color_blend_.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_.logicOpEnable   = VK_FALSE;
        color_blend_.attachmentCount = 1;
        color_blend_.pAttachments    = &color_blend_attachment_;

        shaders_[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaders_[0].pName = "main";

        shaders_[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaders_[1].pName = "main";

        rendering_.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    }

    VkGraphicsPipelineCreateInfo                   info_{};
    VkPipelineVertexInputStateCreateInfo           input_state_{};
    VkPipelineInputAssemblyStateCreateInfo         assembly_{};
    VkPipelineDynamicStateCreateInfo               dynamic_state_{};
    std::array<VkDynamicState, 2>                  dynamic_states_{};
    VkPipelineViewportStateCreateInfo              viewport_{};
    VkPipelineRasterizationStateCreateInfo         rasterization_{};
    VkPipelineMultisampleStateCreateInfo           multisample_{};  // NOLINT
    VkPipelineColorBlendAttachmentState            color_blend_attachment_{};
    VkPipelineColorBlendStateCreateInfo            color_blend_{};
    std::array<VkPipelineShaderStageCreateInfo, 2> shaders_{};  // NOLINT
    VkPipelineRenderingCreateInfo                  rendering_{};
    VkFormat                                       color_attachment_{};
    std::reference_wrapper<VkDevice_T>             dev_;
};
}  // namespace hgn