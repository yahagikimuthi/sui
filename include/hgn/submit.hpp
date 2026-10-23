#pragma once

#include <expected>

#include <vulkan/vulkan.h>

#include "hgn/command.hpp"
#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/swapchain.hpp"
#include "hgn/sync.hpp"

namespace hgn {
class submit_info final {
  public:
    explicit submit_info() noexcept : wait_stage_{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT} {
        native_.sType             = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        native_.pWaitDstStageMask = &wait_stage_;
    }

    submit_info(const submit_info& other) noexcept
        : native_{other.native_},
          wait_semapho_{other.wait_semapho_},
          signal_semapho_{other.signal_semapho_},
          buffer_{other.buffer_},
          wait_stage_{other.wait_stage_} {
        reset();
    }

    auto operator=(const submit_info& other) noexcept -> submit_info& {
        if (this == &other) return *this;

        native_         = other.native_;
        wait_semapho_   = other.wait_semapho_;
        signal_semapho_ = other.signal_semapho_;
        buffer_         = other.buffer_;
        wait_stage_     = other.wait_stage_;

        reset();

        return *this;
    }

    submit_info(submit_info&& other) noexcept
        : native_{other.native_},
          wait_semapho_{other.wait_semapho_},
          signal_semapho_{other.signal_semapho_},
          buffer_{other.buffer_},
          wait_stage_{other.wait_stage_} {
        reset();
    }

    auto operator=(submit_info&& other) noexcept -> submit_info& {
        if (this == &other) return *this;

        native_         = other.native_;
        wait_semapho_   = other.wait_semapho_;
        signal_semapho_ = other.signal_semapho_;
        buffer_         = other.buffer_;
        wait_stage_     = other.wait_stage_;

        reset();

        return *this;
    }

    ~submit_info() noexcept = default;

    auto wait_semaphore(semaphore& semapho) noexcept -> submit_info& {
        wait_semapho_              = &semapho;
        native_.pWaitSemaphores    = &wait_semapho_;
        native_.waitSemaphoreCount = 1;
        return *this;
    }

    auto setting_command_buffer(command_buffer& cmd_buf) noexcept -> submit_info& {
        buffer_                    = &cmd_buf;
        native_.pCommandBuffers    = &buffer_;
        native_.commandBufferCount = 1;
        return *this;
    }

    auto signal_semaphore(semaphore& semapho) noexcept -> submit_info& {
        signal_semapho_              = &semapho;
        native_.pSignalSemaphores    = &signal_semapho_;
        native_.signalSemaphoreCount = 1;
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkSubmitInfo& { return native_; }

  private:
    void reset() noexcept {
        if (native_.pWaitDstStageMask != nullptr) native_.pWaitDstStageMask = &wait_stage_;
    }

    VkSubmitInfo         native_{};
    semaphore*           wait_semapho_{};
    semaphore*           signal_semapho_{};
    command_buffer*      buffer_{};
    VkPipelineStageFlags wait_stage_{};
};

[[nodiscard]] inline auto try_queue_submit(queue& que, const submit_info& info, fence& fen) noexcept
    -> std::expected<void, result> {
    const auto res = vkQueueSubmit(&que, 1, &info.get(), &fen);
    if (res != VK_SUCCESS) return make_result(res);
    return {};
}

class present_info_khr final {
  public:
    explicit present_info_khr() noexcept { native_.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; }

    auto wait_semaphore(semaphore& semapho) noexcept -> present_info_khr& {
        wait_semapho_              = &semapho;
        native_.pWaitSemaphores    = &wait_semapho_;
        native_.waitSemaphoreCount = 1;
        return *this;
    }

    auto setting_swapchain(swapchain_khr& chain) noexcept -> present_info_khr& {
        chain_                 = &chain;
        native_.pSwapchains    = &chain_;
        native_.swapchainCount = 1;
        return *this;
    }

    auto image_indices(const std::uint32_t index) noexcept -> present_info_khr& {
        index_                = index;
        native_.pImageIndices = &index_;
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> const VkPresentInfoKHR& { return native_; }

  private:
    VkPresentInfoKHR native_{};
    swapchain_khr*   chain_{};
    semaphore*       wait_semapho_{};
    std::uint32_t    index_{};
};

[[nodiscard]] inline auto try_queue_present_khr(queue& que, const present_info_khr& info) noexcept
    -> std::expected<void, result> {
    const auto res = vkQueuePresentKHR(&que, &info.get());
    if (res != VK_SUCCESS) return make_result(res);
    return {};
}
}  // namespace hgn