#pragma once

#include <cstdint>
#include <expected>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/descriptor.hpp"
#include "hgn/detail/command/barrier.hpp"
#include "hgn/detail/command/begin_render.hpp"
#include "hgn/enums.hpp"
#include "hgn/raii/command.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/rendering.hpp"

namespace hgn {
class command_buffer final {
  public:
    explicit command_buffer(raw::command_buffer& raw) noexcept : raw_{std::move(raw)} {}

    [[nodiscard]] auto begin() noexcept -> std::expected<void, result> {
        auto info  = VkCommandBufferBeginInfo{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        return make_expected(vkBeginCommandBuffer(&get(), &info));
    }

    [[nodiscard]] auto get_image_memory_barrier_executor() noexcept
        -> image_memory_barrier_executor {
        return image_memory_barrier_executor{get()};
    }

    [[nodiscard]] auto get_begin_rendering_executor() noexcept -> command_begin_rendering_executor {
        return command_begin_rendering_executor{get()};
    }

    void bind_pipeline(graphics_pipeline& pipe) noexcept {
        vkCmdBindPipeline(&get(), VK_PIPELINE_BIND_POINT_GRAPHICS, &pipe.get());
    }

    void bind_vertex_buffer(buffer& buffer) noexcept {
        const auto offset = std::uint64_t{};
        auto*      ptr    = &buffer.get();
        vkCmdBindVertexBuffers(&get(), 0, 1, &ptr, &offset);
    }

    void bind_index_buffer(buffer& buffer) noexcept {
        vkCmdBindIndexBuffer(&get(), &buffer.get(), 0, VK_INDEX_TYPE_UINT32);
    }

    void bind_descriptor_set(pipeline_layout& layout, descriptor_set& set) noexcept {
        auto* ptr = &set.get();
        vkCmdBindDescriptorSets(
            &get(), VK_PIPELINE_BIND_POINT_GRAPHICS, &layout.get(), 0, 1, &ptr, 0, nullptr
        );
    }

    void draw(const std::uint32_t vertex_count, const std::uint32_t instance_count) noexcept {
        vkCmdDraw(&get(), vertex_count, instance_count, 0, 1);
    }

    void draw_indexed(
        const std::uint32_t index_count, const std::uint32_t instance_count
    ) noexcept {
        vkCmdDrawIndexed(&get(), index_count, instance_count, 0, 0, 0);
    }

    void set_viewport(const float width, const float height) noexcept {
        auto vp     = VkViewport{};
        vp.width    = width;
        vp.height   = height;
        vp.minDepth = 0;
        vp.maxDepth = 1;

        vkCmdSetViewport(&get(), 0, 1, &vp);
    }

    void end_rendering() noexcept { vkCmdEndRendering(&get()); }

    [[nodiscard]] auto end() noexcept -> std::expected<void, result> {
        return make_expected(vkEndCommandBuffer(&get()));
    }

    void set_scissor(const std::uint32_t width, const std::uint32_t height) noexcept {
        auto scissor   = VkRect2D{};
        scissor.offset = {.x = 0, .y = 0};
        scissor.extent = {.width = width, .height = height};

        vkCmdSetScissor(&get(), 0, 1, &scissor);
    }

    [[nodiscard]] auto get() noexcept -> raw::command_buffer& { return raw_; }

  private:
    raii::command_buffer raw_;
};

class command_pool final {
  public:
    explicit command_pool(raw::command_pool& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    [[nodiscard]] auto create_buffer() noexcept -> std::expected<command_buffer, result> {
        auto info  = VkCommandBufferAllocateInfo{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

        // コマンドバッファのレベルを示す
        // これは選択可能
        // PRIMARYは直接キューにSubmitできるメイン
        // SECONDARYはプライマリから呼び出して実行するためのサブ
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        // 一度に割り当てるコマンドバッファの数
        // 複数の場合も配列として渡すことが可能
        info.commandBufferCount = 1;
        info.commandPool        = &get();

        auto* raw = static_cast<raw::command_buffer*>(nullptr);
        return make_expected(vkAllocateCommandBuffers(&raw_.init_device(), &info, &raw))
            .transform([&]() noexcept -> command_buffer { return command_buffer{*raw}; });
    }

    [[nodiscard]] auto get() noexcept -> raw::command_pool& { return raw_; }

  private:
    raii::command_pool raw_;
};
}  // namespace hgn