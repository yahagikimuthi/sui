#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <optional>

#include <vulkan/vulkan.h>

#include "hgn/command.hpp"
#include "hgn/enums.hpp"
#include "hgn/raw/raw.hpp"
#include "hgn/swapchain.hpp"
#include "hgn/sync.hpp"

namespace hgn {
class present_executor final {
  public:
    explicit present_executor(raw::queue& queue) noexcept : queue_{std::ref(queue)} {
        info_.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    }

    [[nodiscard]] auto swapchain(swapchain_khr& swapchain) noexcept -> present_executor& {
        swapchain_ = swapchain.get();
        return *this;
    }

    [[nodiscard]] auto semaphore(semaphore& semapho) noexcept -> present_executor& {
        semapho_ = semapho.get();
        return *this;
    }

    [[nodiscard]] auto image_index(const std::uint32_t index) noexcept -> present_executor& {
        image_index_ = index;
        return *this;
    }

    [[nodiscard]] auto present() noexcept -> std::expected<void, result> {
        auto* p_swapchain = static_cast<VkSwapchainKHR_T*>(nullptr);
        auto* p_semapho   = static_cast<raw::semaphore*>(nullptr);

        if (swapchain_) {
            p_swapchain          = &*swapchain_;
            info_.pSwapchains    = &p_swapchain;
            info_.swapchainCount = 1;
        }
        if (semapho_) {
            p_semapho                = &*semapho_;
            info_.pWaitSemaphores    = &p_semapho;
            info_.waitSemaphoreCount = 1;
        }

        info_.pImageIndices = &image_index_;

        return make_expected(vkQueuePresentKHR(&queue_.get(), &info_));
    }

  private:
    VkPresentInfoKHR                   info_{};
    std::uint32_t                      image_index_{};
    std::optional<VkSwapchainKHR_T&>   swapchain_;
    std::optional<raw::semaphore&>     semapho_;
    std::reference_wrapper<raw::queue> queue_;
};

class submitter final {
  public:
    explicit submitter(raw::queue& queue) noexcept : queue_{std::ref(queue)} {
        info_.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    }

    auto command_buffer(command_buffer& buffer) noexcept -> submitter& {
        cmd_buffer_ = buffer.get();
        return *this;
    }

    auto wait_semaphore(semaphore& wait) noexcept -> submitter& {
        wait_ = wait.get();
        return *this;
    }

    auto signal_semaphore(semaphore& signal) noexcept -> submitter& {
        signal_ = signal.get();
        return *this;
    }

    [[nodiscard]] auto submit(fence& fence) noexcept -> std::expected<void, result> {
        auto wait_stage = static_cast<std::uint32_t>(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        info_.pWaitDstStageMask  = &wait_stage;
        auto* p_cmd_buffer       = static_cast<raw::command_buffer*>(nullptr);
        auto* p_wait_semaphore   = static_cast<raw::semaphore*>(nullptr);
        auto* p_signal_semaphore = static_cast<raw::semaphore*>(nullptr);

        if (cmd_buffer_) {
            p_cmd_buffer             = &*cmd_buffer_;
            info_.commandBufferCount = 1;
            info_.pCommandBuffers    = &p_cmd_buffer;
        }
        if (wait_) {
            p_wait_semaphore         = &*wait_;
            info_.waitSemaphoreCount = 1;
            info_.pWaitSemaphores    = &p_wait_semaphore;
        }
        if (signal_) {
            p_signal_semaphore         = &*signal_;
            info_.signalSemaphoreCount = 1;
            info_.pSignalSemaphores    = &p_signal_semaphore;
        }

        return make_expected(vkQueueSubmit(&queue_.get(), 1, &info_, &fence.get()));
    }

  private:
    VkSubmitInfo                        info_{};
    std::optional<raw::command_buffer&> cmd_buffer_;
    std::optional<raw::semaphore&>      wait_;
    std::optional<raw::semaphore&>      signal_;
    std::reference_wrapper<raw::queue>  queue_;
};
}  // namespace hgn