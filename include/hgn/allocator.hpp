#pragma once

#include <expected>
#include <utility>

#include <hgn/detail/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/enums.hpp"
#include "hgn/raii/command.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class allocator final {
    friend class allocator_creator;

  public:
    [[nodiscard]] auto create_buffer(const std::uint64_t size) -> std::expected<buffer, result> {
        auto info  = VkBufferCreateInfo{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // キューファミリーをどのように共有するか。以下を設定できる
        // EXCLUSIVE: 単一キューファミリーからアクセス可能。最も一般的で効率的
        // CONCURRENT: 複数キューファミリーから同時アクセス可能
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // バッファの使用目的。現在はVB固定だが、今後IBやUBが必要になる場合カスタムする
        info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        info.size  = size;
        // info.(p)queueFamilyIndex(Count)があるが、EXCLUSIVEの場合は0で良い

        // それぞれの解説
        // flags: アロケーションの方法や動作を制御するビット
        // ほかにもMAPPED_BIT(永続化マップ), DEDICATED_MEMORY_BIT(非共有ビット)などがある
        // usage: メモリの用途、AUTOが推奨
        // requiredFlags: メモリタイププロパティへの要件。usageがAUTOの場合0で良い
        // 優先するメモリプロパティを指定する。通常は0で良い
        // memoryTypeBits: 受け入れ可能なメモリタイプ。特別な理由がない場合0で良い
        // pool: どのVmaPoolから切り出すかを指定する。nullの場合はVMA側から割当
        // pUserData: アロケーションのユーザー定義ハンドラ.glfwSetWindowUserPointerみたいなもの
        // priority: 他のアロケーションの重要度。メモリ逼迫時などのヒントになる
        // minAlignment: 追加の最小アライメントを指定。0か2^nを指定。特別な理由がないなら0
        const auto alloc_create_info = VmaAllocationCreateInfo{
            .flags          = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage          = VMA_MEMORY_USAGE_AUTO,
            .requiredFlags  = 0,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool           = nullptr,
            .pUserData      = nullptr,
            .priority       = 0,
            .minAlignment   = 0
        };

        auto*      raw            = static_cast<raw::buffer*>(nullptr);
        auto*      raw_allocation = static_cast<raw::allocation*>(nullptr);
        auto       alloc_info     = VmaAllocationInfo{};
        const auto res =
            vmaCreateBuffer(&get(), &info, &alloc_create_info, &raw, &raw_allocation, &alloc_info);

        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        assert(raw_allocation != nullptr);
        return buffer{*raw, get(), *raw_allocation, alloc_info.size};
    }

    [[nodiscard]] auto get() noexcept -> raw::allocator& { return raw_; }

  private:
    explicit allocator(raw::allocator& raw) noexcept : raw_{std::move(raw)} {}

    raii::allocator raw_;
};
}  // namespace hgn