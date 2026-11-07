#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/command.hpp"
#include "hgn/descriptor.hpp"
#include "hgn/descriptor_pool.hpp"
#include "hgn/detail/device/image_view.hpp"
#include "hgn/detail/device/swapchain.hpp"
#include "hgn/detail/pipeline.hpp"
#include "hgn/enums.hpp"
#include "hgn/queue.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/rendering.hpp"
#include "hgn/sync.hpp"

namespace hgn {
class device final {
  public:
    explicit device(raw::device& raw) noexcept : raw_{std::move(raw)} {}

    [[nodiscard]] auto get_queue(const queue_family& family) noexcept -> queue {
        // 指定されたキューファミリーの0番キューを取得する
        auto* raw = static_cast<raw::queue*>(nullptr);
        vkGetDeviceQueue(&get(), family.index(), 0, &raw);

        assert(raw != nullptr);
        return queue{*raw};
    }

    [[nodiscard]] auto create_semaphore() noexcept -> std::expected<semaphore, result> {
        auto info  = VkSemaphoreCreateInfo{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        // 他の値であるpNextとflagsは基本的にnullptr, 0で十分

        auto* raw = static_cast<raw::semaphore*>(nullptr);
        return make_expected(vkCreateSemaphore(&get(), &info, nullptr, &raw))
            .transform([&]() noexcept -> semaphore { return semaphore{*raw, get()}; });
    }

    [[nodiscard]] auto create_fence() noexcept -> std::expected<fence, result> {
        auto info  = VkFenceCreateInfo{};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // flagsは0の場合、初期化時にGPU処理が終了する保証がないため、最初の描画で待機する可能性がある
        // 以下の定数を入力した場合、同期が終了した状態で生成されるため、待機することがない
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        auto* raw = static_cast<raw::fence*>(nullptr);
        return make_expected(vkCreateFence(&get(), &info, nullptr, &raw))
            .transform([&]() noexcept -> fence { return fence{*raw, get()}; });
    }

    [[nodiscard]] auto get_swapchain_creator() noexcept -> swapchain_khr_creator {
        return swapchain_khr_creator{get()};
    }

    [[nodiscard]] auto create_command_pool(const queue_family& family
    ) noexcept -> std::expected<command_pool, result> {
        auto info  = VkCommandPoolCreateInfo{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

        // この定数はプールを再利用することを示す
        // 再利用しない場合はTRANSIENT_BITを指定する
        // 他にも、PROTECTED_BITは著作権保護コンテンツなど保護メモリを扱う場合に指定する
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        info.queueFamilyIndex = family.index();

        auto* raw = static_cast<raw::command_pool*>(nullptr);
        return make_expected(vkCreateCommandPool(&get(), &info, nullptr, &raw))
            .transform([&]() noexcept -> command_pool { return command_pool{*raw, get()}; });
    }

    [[nodiscard]] auto get_image_view_creator() noexcept -> image_view_creator {
        return image_view_creator{get()};
    }

    [[nodiscard]] auto create_shader(const std::span<const char> codes
    ) noexcept -> std::expected<shader_module, result> {
        auto info     = VkShaderModuleCreateInfo{};
        info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = codes.size();

        // charとunsigned intのビット再解釈は厳密エイリアス規則の例外
        info.pCode = reinterpret_cast<const std::uint32_t*>(codes.data());  // NOLINT

        auto* raw = static_cast<raw::shader_module*>(nullptr);
        return make_expected(vkCreateShaderModule(&get(), &info, nullptr, &raw))
            .transform([&]() noexcept -> shader_module { return shader_module{*raw, get()}; });
    }

    [[nodiscard]] auto create_pipeline_layout() noexcept -> std::expected<pipeline_layout, result> {
        // シェーダーのソースコード(layoutキーワード)の解釈を伝える
        auto info                   = VkPipelineLayoutCreateInfo{};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount         = 0;
        info.pushConstantRangeCount = 0;

        auto* raw = static_cast<raw::pipeline_layout*>(nullptr);
        return make_expected(vkCreatePipelineLayout(&get(), &info, nullptr, &raw))
            .transform([&]() noexcept -> pipeline_layout { return pipeline_layout{*raw, get()}; });
    }

    [[nodiscard]] auto create_pipeline_layout(descriptor_set_layout& set
    ) noexcept -> std::expected<pipeline_layout, result> {
        auto* ptr                   = &set.get();
        auto  info                  = VkPipelineLayoutCreateInfo{};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount         = 1;
        info.pSetLayouts            = &ptr;
        info.pushConstantRangeCount = 0;

        auto* raw = static_cast<raw::pipeline_layout*>(nullptr);
        return make_expected(vkCreatePipelineLayout(&get(), &info, nullptr, &raw))
            .transform([&]() noexcept -> pipeline_layout { return pipeline_layout{*raw, get()}; });
    }

    [[nodiscard]] auto get_graphics_pipeline_creator() noexcept -> graphics_pipeline_creator {
        return graphics_pipeline_creator{get()};
    }

    [[nodiscard]] auto create_descriptor_set_layout() noexcept
        -> std::expected<descriptor_set_layout, result> {
        auto binding            = VkDescriptorSetLayoutBinding{};
        binding.binding         = 0;
        binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.descriptorCount = 1;
        binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

        auto info         = VkDescriptorSetLayoutCreateInfo{};
        info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = 1;
        info.pBindings    = &binding;

        auto* raw = static_cast<raw::descriptor_set_layout*>(nullptr);
        return make_expected(vkCreateDescriptorSetLayout(&get(), &info, nullptr, &raw))
            .transform([&]() noexcept -> descriptor_set_layout {
                return descriptor_set_layout{*raw, get()};
            });
    }

    [[nodiscard]] auto create_descriptor_pool() noexcept -> std::expected<descriptor_pool, result> {
        auto size            = VkDescriptorPoolSize{};
        size.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        size.descriptorCount = 1;

        auto info          = VkDescriptorPoolCreateInfo{};
        info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.poolSizeCount = 1;
        info.pPoolSizes    = &size;
        info.maxSets       = 1;

        auto* raw = static_cast<raw::descriptor_pool*>(nullptr);
        return make_expected(vkCreateDescriptorPool(&get(), &info, nullptr, &raw))
            .transform([&]() noexcept -> descriptor_pool { return descriptor_pool{*raw, get()}; });
    }

    [[nodiscard]] auto wait_idle() noexcept -> std::expected<void, result> {
        return make_expected(vkDeviceWaitIdle(&get()));
    }

    [[nodiscard]] auto get() noexcept -> raw::device& { return raw_; }

  private:
    raii::device raw_;
};
}  // namespace hgn