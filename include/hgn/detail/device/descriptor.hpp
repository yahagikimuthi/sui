#pragma once

#include <cassert>
#include <expected>
#include <functional>
#include <optional>

#include "hgn/descriptor.hpp"
#include "hgn/enums.hpp"
#include "hgn/raw/raw.hpp"

namespace hgn {
class descriptor_set_getter final {
    friend class device;

  public:
    auto pool(descriptor_pool& pool) noexcept -> descriptor_set_getter& {
        info_.descriptorPool = &pool.get();
        return *this;
    }

    auto layout(descriptor_set_layout& layout) noexcept -> descriptor_set_getter& {
        layout_ = layout.get();
        return *this;
    }

    auto get() noexcept -> std::expected<descriptor_set, result> {
        auto* p_layout = static_cast<raw::descriptor_set_layout*>(nullptr);
        if (layout_) {
            info_.descriptorSetCount = 1;
            p_layout                 = &*layout_;
            info_.pSetLayouts        = &p_layout;
        }
        auto*      raw = static_cast<raw::descriptor_set*>(nullptr);
        const auto res = vkAllocateDescriptorSets(&dev_.get(), &info_, &raw);
        if (res != VK_SUCCESS) return make_unexpected(res);

        assert(raw != nullptr);
        return descriptor_set{*raw, dev_};
    }

  private:
    explicit descriptor_set_getter(raw::device& dev) noexcept : dev_{std::ref(dev)} {
        info_.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    }

    VkDescriptorSetAllocateInfo                info_{};
    std::optional<raw::descriptor_set_layout&> layout_;
    std::reference_wrapper<raw::device>        dev_;
};
}  // namespace hgn