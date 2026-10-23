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

#include <vulkan/vulkan.h>

#include "hgn/command.hpp"
#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/shader.hpp"

namespace hgn {
class vertex_input_binding_description final {
  public:
    explicit vertex_input_binding_description() noexcept {
        native_.binding   = 0;
        native_.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }

    auto stride(const std::uint32_t str) noexcept -> vertex_input_binding_description& {
        native_.stride = str;
        return *this;
    }

  private:
    VkVertexInputBindingDescription native_{};
};

class vertex_input_attribute_description final {
  public:
    explicit vertex_input_attribute_description() noexcept { native_.binding = 0; }

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
        const std::span<const vertex_input_attribute_description> descriptions
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

    [[nodiscard]] auto get() const noexcept -> const VkPipelineVertexInputStateCreateInfo& {
        return native_;
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
        native_.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        native_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        native_.primitiveRestartEnable = VK_FALSE;
    }

  private:
    VkPipelineInputAssemblyStateCreateInfo native_{};
};

class pipeline_dynamic_state_create_info final {
  public:
    explicit pipeline_dynamic_state_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    }

    pipeline_dynamic_state_create_info(const pipeline_dynamic_state_create_info& other) noexcept
        : native_{other.native_}, states_{other.states_} {
        reset();
    }

    auto operator=(const pipeline_dynamic_state_create_info& other) noexcept
        -> pipeline_dynamic_state_create_info& {
        if (this == &other) return *this;

        native_ = other.native_;
        states_ = other.states_;

        reset();
        return *this;
    }

    pipeline_dynamic_state_create_info(pipeline_dynamic_state_create_info&& other) noexcept
        : native_{other.native_}, states_{std::move(other.states_)} {
        reset();
    }

    auto operator=(pipeline_dynamic_state_create_info&& other) noexcept
        -> pipeline_dynamic_state_create_info& {
        if (this == &other) return *this;

        native_ = other.native_;
        states_ = std::move(other.states_);

        reset();
        return *this;
    }

    ~pipeline_dynamic_state_create_info() noexcept = default;

    auto dynamic_states(const std::span<const dynamic_state> states) noexcept
        -> pipeline_dynamic_state_create_info& {
        states_.reserve(states.size());
        states_.assign_range(
            states | std::views::transform([](dynamic_state state) noexcept -> VkDynamicState {
                return static_cast<VkDynamicState>(state);
            })
        );

        native_.dynamicStateCount = static_cast<std::uint32_t>(states_.size());
        native_.pDynamicStates    = states_.data();
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkPipelineDynamicStateCreateInfo& {
        return native_;
    }

  private:
    void reset() noexcept {
        if (native_.pDynamicStates != nullptr) native_.pDynamicStates = states_.data();
    }

    VkPipelineDynamicStateCreateInfo native_{};
    std::vector<VkDynamicState>      states_;
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
        native_.sampleShadingEnable  = VK_FALSE;
        native_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
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

    pipeline_color_blend_state_create_info(
        const pipeline_color_blend_state_create_info& other
    ) noexcept
        : native_{other.native_}, attachment_{other.attachment_} {
        reset();
    }

    auto operator=(const pipeline_color_blend_state_create_info& other) noexcept
        -> pipeline_color_blend_state_create_info& {
        if (this == &other) return *this;

        native_     = other.native_;
        attachment_ = other.attachment_;

        reset();
        return *this;
    }

    pipeline_color_blend_state_create_info(pipeline_color_blend_state_create_info&& other) noexcept
        : native_{other.native_}, attachment_{other.attachment_} {}

    auto operator=(pipeline_color_blend_state_create_info&& other) noexcept
        -> pipeline_color_blend_state_create_info& {
        if (this == &other) return *this;

        native_     = other.native_;
        attachment_ = other.attachment_;

        reset();
        return *this;
    }

    ~pipeline_color_blend_state_create_info() noexcept = default;

    auto attachment(const pipeline_color_blend_attachment_state& state) noexcept
        -> pipeline_color_blend_state_create_info& {
        attachment_             = std::bit_cast<VkPipelineColorBlendAttachmentState>(state);
        native_.attachmentCount = 1;
        native_.pAttachments    = &attachment_;
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkPipelineColorBlendStateCreateInfo& {
        return native_;
    }

  private:
    void reset() noexcept {
        if (native_.pAttachments != nullptr) native_.pAttachments = &attachment_;
    }

    VkPipelineColorBlendStateCreateInfo native_{};
    VkPipelineColorBlendAttachmentState attachment_{};
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

  private:
    VkPipelineLayoutCreateInfo native_{};
};

[[nodiscard]] inline auto try_make_pipeline_layout(
    device& dev, const pipeline_layout_create_info& info
) noexcept -> std::expected<std::reference_wrapper<pipeline_layout>, result> {
    auto*      layout_ptr  = static_cast<pipeline_layout*>(nullptr);
    auto       native_info = std::bit_cast<VkPipelineLayoutCreateInfo>(info);
    const auto res         = vkCreatePipelineLayout(&dev, &native_info, nullptr, &layout_ptr);

    if (res != VK_SUCCESS) return make_result(res);

    assert(layout_ptr != nullptr);
    return std::ref(*layout_ptr);
}

class pipeline_rendering_create_info final {
  public:
    explicit pipeline_rendering_create_info() noexcept {
        native_.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    }

    pipeline_rendering_create_info(const pipeline_rendering_create_info& other) noexcept
        : native_{other.native_}, format_{other.format_} {
        reset();
    }

    auto operator=(const pipeline_rendering_create_info& other) noexcept
        -> pipeline_rendering_create_info& {
        if (this == &other) return *this;

        native_ = other.native_;
        format_ = other.format_;

        reset();
        return *this;
    }

    pipeline_rendering_create_info(pipeline_rendering_create_info&& other) noexcept
        : native_{other.native_}, format_{other.format_} {
        reset();
    }

    auto operator=(pipeline_rendering_create_info&& other) noexcept
        -> pipeline_rendering_create_info& {
        if (this == &other) return *this;

        native_ = other.native_;
        format_ = other.format_;

        reset();
        return *this;
    }

    ~pipeline_rendering_create_info() noexcept = default;

    auto color_attachment_format(const format f) noexcept -> pipeline_rendering_create_info& {
        format_                         = static_cast<VkFormat>(f);
        native_.colorAttachmentCount    = 1;
        native_.pColorAttachmentFormats = &format_;
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkPipelineRenderingCreateInfo& {
        return native_;
    }

  private:
    void reset() noexcept {
        if (native_.pColorAttachmentFormats != nullptr) native_.pColorAttachmentFormats = &format_;
    }

    VkPipelineRenderingCreateInfo native_{};
    VkFormat                      format_{static_cast<VkFormat>(0)};
};

class graphics_pipeline_create_info final {
  public:
    explicit graphics_pipeline_create_info() noexcept {
        native_.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        native_.renderPass = nullptr;
        native_.subpass    = 0;
    }

    graphics_pipeline_create_info(const graphics_pipeline_create_info& other)
        : native_{other.native_}, rendering_info_{other.rendering_info_} {
        reset();
    }

    auto operator=(const graphics_pipeline_create_info& other) noexcept
        -> graphics_pipeline_create_info& {
        if (this == &other) return *this;

        native_         = other.native_;
        rendering_info_ = other.rendering_info_;

        reset();
        return *this;
    }

    graphics_pipeline_create_info(graphics_pipeline_create_info&& other) noexcept
        : native_{other.native_}, rendering_info_{other.rendering_info_} {
        reset();
    }

    auto operator=(graphics_pipeline_create_info&& other) noexcept
        -> graphics_pipeline_create_info& {
        if (this == &other) return *this;

        native_         = other.native_;
        rendering_info_ = other.rendering_info_;

        reset();
        return *this;
    }

    ~graphics_pipeline_create_info() noexcept = default;

    auto rendering_info(const pipeline_rendering_create_info& info) noexcept
        -> graphics_pipeline_create_info& {
        rendering_info_ = info.get();
        native_.pNext   = &rendering_info_;
        return *this;
    }

    auto stages(const std::span<const pipeline_shader_stage_create_info> stage_infos) noexcept
        -> graphics_pipeline_create_info& {
        shader_stages_.reserve(stage_infos.size());
        shader_stages_.assign_range(
            stage_infos | std::views::transform(
                              [](const pipeline_shader_stage_create_info& info) noexcept
                                  -> VkPipelineShaderStageCreateInfo {
                                  return std::bit_cast<VkPipelineShaderStageCreateInfo>(info);
                              }
                          )
        );

        native_.stageCount = static_cast<std::uint32_t>(shader_stages_.size());
        native_.pStages    = shader_stages_.data();
        return *this;
    }

    auto vertex_input_state(const pipeline_vertex_input_state_create_info& info) noexcept
        -> graphics_pipeline_create_info& {
        vertex_input_info_        = info.get();
        native_.pVertexInputState = &vertex_input_info_;
        return *this;
    }

    auto input_assembly_state(const pipeline_input_assembly_state_create_info& info) noexcept
        -> graphics_pipeline_create_info& {
        input_assembly_             = std::bit_cast<VkPipelineInputAssemblyStateCreateInfo>(info);
        native_.pInputAssemblyState = &input_assembly_;
        return *this;
    }

    auto viewport_state(const pipeline_viewport_state_create_info& info) noexcept
        -> graphics_pipeline_create_info& {
        viewport_state_        = std::bit_cast<VkPipelineViewportStateCreateInfo>(info);
        native_.pViewportState = &viewport_state_;
        return *this;
    }

    auto rasterization_state(const pipeline_rasterization_state_create_info& info) noexcept
        -> graphics_pipeline_create_info& {
        rasterizer_                 = std::bit_cast<VkPipelineRasterizationStateCreateInfo>(info);
        native_.pRasterizationState = &rasterizer_;
        return *this;
    }

    auto multisample_state(const pipeline_multisample_state_create_info& info) noexcept
        -> graphics_pipeline_create_info& {
        multisampling_            = std::bit_cast<VkPipelineMultisampleStateCreateInfo>(info);
        native_.pMultisampleState = &multisampling_;
        return *this;
    }

    auto color_blend_state(const pipeline_color_blend_state_create_info& info) noexcept
        -> graphics_pipeline_create_info& {
        color_blend_             = info.get();
        native_.pColorBlendState = &color_blend_;
        return *this;
    }

    auto dynamic_states(const pipeline_dynamic_state_create_info& info) noexcept
        -> graphics_pipeline_create_info& {
        dynamic_state_        = info.get();
        native_.pDynamicState = &dynamic_state_;
        return *this;
    }

    auto layout(pipeline_layout& ref) noexcept -> graphics_pipeline_create_info& {
        native_.layout = &ref;
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkGraphicsPipelineCreateInfo& {
        return native_;
    }

  private:
    void reset() noexcept {
        if (native_.pNext != nullptr) native_.pNext = &rendering_info_;
        if (native_.pStages != nullptr) native_.pStages = shader_stages_.data();
        if (native_.pVertexInputState != nullptr) native_.pVertexInputState = &vertex_input_info_;
        if (native_.pInputAssemblyState != nullptr) native_.pInputAssemblyState = &input_assembly_;
    }

    VkGraphicsPipelineCreateInfo                 native_{};
    VkPipelineRenderingCreateInfo                rendering_info_{};
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
    VkPipelineVertexInputStateCreateInfo         vertex_input_info_{};
    VkPipelineInputAssemblyStateCreateInfo       input_assembly_{};
    VkPipelineViewportStateCreateInfo            viewport_state_{};
    VkPipelineRasterizationStateCreateInfo       rasterizer_{};
    VkPipelineMultisampleStateCreateInfo         multisampling_{};  // NOLINT
    VkPipelineColorBlendStateCreateInfo          color_blend_{};
    VkPipelineDynamicStateCreateInfo             dynamic_state_{};
};

using pipeline = VkPipeline_T;

[[nodiscard]] inline auto try_make_graphics_pipeline(
    device& dev, const graphics_pipeline_create_info& info
) noexcept -> std::expected<std::reference_wrapper<pipeline>, result> {
    auto* pipe_ptr = static_cast<pipeline*>(nullptr);

    const auto res = vkCreateGraphicsPipelines(&dev, nullptr, 1, &info.get(), nullptr, &pipe_ptr);
    if (res != VK_SUCCESS) return make_result(res);

    assert(pipe_ptr != nullptr);
    return std::ref(*pipe_ptr);
}

inline void destroy_pipeline(device& dev, pipeline& target) noexcept {
    vkDestroyPipeline(&dev, &target, nullptr);
}

inline void command_bind_pipeline(command_buffer& buf, pipeline& pipe) noexcept {
    vkCmdBindPipeline(&buf, VK_PIPELINE_BIND_POINT_GRAPHICS, &pipe);
}
}  // namespace hgn