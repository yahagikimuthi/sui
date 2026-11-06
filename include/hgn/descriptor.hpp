#pragma once

#include <functional>
#include <utility>

#include "hgn/buffer.hpp"
#include "hgn/raii/rendering.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class descriptor_set_layout final {
    friend class device;

  public:
    [[nodiscard]] auto get() noexcept -> raw::descriptor_set_layout& { return raw_; }

  private:
    explicit descriptor_set_layout(raw::descriptor_set_layout& raw, raw::device& dev) noexcept
        : raw_{std::move(raw), dev} {}

    raii::descriptor_set_layout raw_;
};

class descriptor_set final {
    friend class descriptor_pool;

  public:
    void update(buffer& buffer) noexcept {
        auto info   = VkDescriptorBufferInfo{};
        info.buffer = &buffer.get();
        info.offset = 0;
        info.range  = buffer.size();

        auto write            = VkWriteDescriptorSet{};
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet          = &get();
        write.dstBinding      = 0;
        write.dstArrayElement = 0;
        write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo     = &info;

        vkUpdateDescriptorSets(&dev_.get(), 1, &write, 0, nullptr);
    }

    [[nodiscard]] auto get() noexcept -> raw::descriptor_set& { return raw_; }

  private:
    explicit descriptor_set(raw::descriptor_set& raw, raw::device& dev) noexcept
        : raw_{raw}, dev_{std::ref(dev)} {}

    raii::descriptor_set                raw_;
    std::reference_wrapper<raw::device> dev_;
};
}  // namespace hgn