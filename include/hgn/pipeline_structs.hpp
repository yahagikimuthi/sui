#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>

#include <vulkan/vulkan.h>

#include "hgn/enums.hpp"
#include "hgn/pipeline.hpp"
#include "hgn/pipeline_layout.hpp"
#include "hgn/raw.hpp"
#include "hgn/shader.hpp"

namespace hgn {
class graphics_pipeline_builder final {
    friend class device;

  public:
    auto vertex_binding(const std::uint32_t stride) noexcept -> graphics_pipeline_builder& {
        vertex_binding_.stride = stride;
        return *this;
    }

    auto vertex_offsets(const std::uint32_t pos0, const std::uint32_t pos1) noexcept
        -> graphics_pipeline_builder& {
        vertex_attributes_[0].offset = pos0;
        vertex_attributes_[1].offset = pos1;
        return *this;
    }

    auto color_attachment(const format fmt) noexcept -> graphics_pipeline_builder& {
        color_attachment_ = static_cast<VkFormat>(fmt);
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
        vertex_state_.vertexBindingDescriptionCount   = 1;
        vertex_state_.pVertexBindingDescriptions      = &vertex_binding_;
        vertex_state_.vertexAttributeDescriptionCount = 2;
        vertex_state_.pVertexAttributeDescriptions    = vertex_attributes_.data();

        dynamic_state_.dynamicStateCount = 2;
        dynamic_state_.pDynamicStates    = dynamic_states_.data();

        color_blend_.attachmentCount = 1;
        color_blend_.pAttachments    = &color_blend_attachment_;

        rendering_.colorAttachmentCount    = 1;
        rendering_.pColorAttachmentFormats = &color_attachment_;

        info_.pVertexInputState   = &vertex_state_;
        info_.pInputAssemblyState = &assembly_;
        info_.pDynamicState       = &dynamic_state_;
        info_.pViewportState      = &viewport_;
        info_.pRasterizationState = &rasterization_;
        info_.pMultisampleState   = &multisample_;
        info_.pColorBlendState    = &color_blend_;
        info_.stageCount          = 2;
        info_.pStages             = shaders_.data();
        info_.pNext               = &rendering_;

        auto*      raw = static_cast<raw::pipeline*>(nullptr);
        const auto res = vkCreateGraphicsPipelines(&dev_.get(), nullptr, 1, &info_, nullptr, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return graphics_pipeline{*raw, dev_.get()};
    }

  private:
    explicit graphics_pipeline_builder(raw::device& dev) noexcept : dev_{std::ref(dev)} {
        info_.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info_.renderPass = nullptr;
        info_.subpass    = 0;

        vertex_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        vertex_binding_.binding   = 0;
        vertex_binding_.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        vertex_attributes_[0].binding  = 0;
        vertex_attributes_[0].location = 0;
        vertex_attributes_[0].format   = VK_FORMAT_R32G32_SFLOAT;

        vertex_attributes_[1].binding  = 0;
        vertex_attributes_[1].location = 1;
        vertex_attributes_[1].format   = VK_FORMAT_R32G32_SFLOAT;

        assembly_.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        assembly_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        assembly_.primitiveRestartEnable = VK_FALSE;

        dynamic_states_      = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
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

        color_blend_.sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_.logicOpEnable = VK_FALSE;

        shaders_[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaders_[0].pName = "main";

        shaders_[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaders_[1].pName = "main";

        rendering_.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    }

    VkGraphicsPipelineCreateInfo                     info_{};
    VkPipelineVertexInputStateCreateInfo             vertex_state_{};
    VkVertexInputBindingDescription                  vertex_binding_{};
    std::array<VkVertexInputAttributeDescription, 2> vertex_attributes_{};
    VkPipelineInputAssemblyStateCreateInfo           assembly_{};
    VkPipelineDynamicStateCreateInfo                 dynamic_state_{};
    std::array<VkDynamicState, 2>                    dynamic_states_{};
    VkPipelineViewportStateCreateInfo                viewport_{};
    VkPipelineRasterizationStateCreateInfo           rasterization_{};
    VkPipelineMultisampleStateCreateInfo             multisample_{};  // NOLINT
    VkPipelineColorBlendAttachmentState              color_blend_attachment_{};
    VkPipelineColorBlendStateCreateInfo              color_blend_{};
    std::array<VkPipelineShaderStageCreateInfo, 2>   shaders_{};  // NOLINT
    VkPipelineRenderingCreateInfo                    rendering_{};
    VkFormat                                         color_attachment_{};
    std::reference_wrapper<raw::device>              dev_;
};
}  // namespace hgn