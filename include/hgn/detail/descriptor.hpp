#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <vector>

#include <vulkan/vulkan.h>

#include "hgn/buffer.hpp"
#include "hgn/enums.hpp"
#include "hgn/image.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class descriptor_set_writer;
class descriptor_set_write_pusher final {
  public:
    explicit descriptor_set_write_pusher(
        descriptor_set_writer&             writer,
        std::vector<VkWriteDescriptorSet>& vec,
        VkDescriptorBufferInfo&            buffer_info,
        VkDescriptorImageInfo&             image_info,
        raw::descriptor_set&               set
    )
        : vec_{std::ref(vec)},
          buffer_info_{std::ref(buffer_info)},
          image_info_{std::ref(image_info)},
          writer_{std::ref(writer)} {
        write_.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_.dstSet          = &set;
        write_.dstBinding      = 0;
        write_.dstArrayElement = 0;
        write_.descriptorCount = 1;

        buffer_info_.get().offset = 0;

        image_info_.get().imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    auto buffer(buffer& buffer) noexcept -> descriptor_set_write_pusher& {
        buffer_info_.get().buffer = &buffer.get();
        buffer_info_.get().range  = buffer.size();
        return *this;
    }

    auto type(const descriptor_type type) noexcept -> descriptor_set_write_pusher& {
        write_.descriptorType = static_cast<VkDescriptorType>(type);
        return *this;
    }

    auto image(image_view& view, sampler& sampler) noexcept -> descriptor_set_write_pusher& {
        image_info_.get().imageView = &view.get();
        image_info_.get().sampler   = &sampler.get();
        return *this;
    }

    auto push() noexcept -> descriptor_set_writer& {
        if (buffer_info_.get().buffer != nullptr) write_.pBufferInfo = &buffer_info_.get();
        if (image_info_.get().imageView != nullptr) write_.pImageInfo = &image_info_.get();

        auto& vec = vec_.get();

        vec.emplace_back(write_);
        return writer_;
    }

  private:
    VkWriteDescriptorSet                                      write_{};
    std::reference_wrapper<std::vector<VkWriteDescriptorSet>> vec_;
    std::reference_wrapper<VkDescriptorBufferInfo>            buffer_info_;
    std::reference_wrapper<VkDescriptorImageInfo>             image_info_;
    std::reference_wrapper<descriptor_set_writer>             writer_;
};

class descriptor_set_writer final {
  public:
    explicit descriptor_set_writer(raw::descriptor_set& set, raw::device& dev) noexcept
        : set_{std::ref(set)}, dev_{std::ref(dev)} {}

    [[nodiscard]] auto get_pusher() noexcept -> descriptor_set_write_pusher {
        return descriptor_set_write_pusher{
            *this, writes_, buffer_infos_.emplace_back(), image_infos_.emplace_back(), set_
        };
    }

    void write() noexcept {
        vkUpdateDescriptorSets(
            &dev_.get(), static_cast<std::uint32_t>(writes_.size()), writes_.data(), 0, nullptr
        );
    }

  private:
    std::vector<VkWriteDescriptorSet>           writes_;
    std::deque<VkDescriptorBufferInfo>          buffer_infos_;
    std::deque<VkDescriptorImageInfo>           image_infos_;
    std::reference_wrapper<raw::descriptor_set> set_;
    std::reference_wrapper<raw::device>         dev_;
};
}  // namespace hgn