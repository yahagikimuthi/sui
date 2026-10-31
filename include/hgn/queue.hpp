#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/command.hpp"
#include "hgn/device.hpp"
#include "hgn/enums.hpp"
#include "hgn/raii/hardware.hpp"
#include "hgn/swapchain.hpp"
#include "hgn/sync.hpp"

namespace hgn {
class present_executor final {
    friend class queue;

  public:
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

    [[nodiscard]] auto execute() noexcept -> std::expected<void, result> {
        auto* p_swapchain = static_cast<VkSwapchainKHR_T*>(nullptr);
        auto* p_semapho   = static_cast<VkSemaphore_T*>(nullptr);

        if (swapchain_) p_swapchain = &*swapchain_;
        if (semapho_) p_semapho = &*semapho_;

        info_.pSwapchains        = &p_swapchain;
        info_.swapchainCount     = 1;
        info_.pWaitSemaphores    = &p_semapho;
        info_.waitSemaphoreCount = 1;
        info_.pImageIndices      = &image_index_;

        const auto res = vkQueuePresentKHR(&queue_.get(), &info_);
        if (res != VK_SUCCESS) return make_unexpected(res);

        return {};
    }

  private:
    explicit present_executor(VkQueue_T& queue) noexcept : queue_{std::ref(queue)} {
        info_.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    }

    VkPresentInfoKHR                  info_{};
    std::uint32_t                     image_index_{};
    std::optional<VkSwapchainKHR_T&>  swapchain_;
    std::optional<VkSemaphore_T&>     semapho_;
    std::reference_wrapper<VkQueue_T> queue_;
};

class submit_executor final {
    friend class queue;

  public:
    auto command_buffer(command_buffer& buffer) noexcept -> submit_executor& {
        cmd_buffer_ = buffer.get();
        return *this;
    }

    auto wait_semaphore(semaphore& wait) noexcept -> submit_executor& {
        wait_ = wait.get();
        return *this;
    }

    auto signal_semaphore(semaphore& signal) noexcept -> submit_executor& {
        signal_ = signal.get();
        return *this;
    }

    [[nodiscard]] auto submit(fence& fence) noexcept -> std::expected<void, result> {
        auto wait_stage = static_cast<std::uint32_t>(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        info_.pWaitDstStageMask  = &wait_stage;
        auto* p_cmd_buffer       = static_cast<VkCommandBuffer_T*>(nullptr);
        auto* p_wait_semaphore   = static_cast<VkSemaphore_T*>(nullptr);
        auto* p_signal_semaphore = static_cast<VkSemaphore_T*>(nullptr);

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

        const auto res = vkQueueSubmit(&queue_.get(), 1, &info_, &fence.get());
        if (res != VK_SUCCESS) return make_unexpected(res);

        return {};
    }

  private:
    explicit submit_executor(VkQueue_T& queue) noexcept : queue_{std::ref(queue)} {
        info_.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    }

    VkSubmitInfo                      info_{};
    std::optional<VkCommandBuffer_T&> cmd_buffer_;
    std::optional<VkSemaphore_T&>     wait_;
    std::optional<VkSemaphore_T&>     signal_;
    std::reference_wrapper<VkQueue_T> queue_;
};

class queue final {
    friend class device;

  public:
    queue(const queue&) noexcept                    = delete;
    auto operator=(const queue&) noexcept -> queue& = delete;
    queue(queue&&) noexcept                         = default;
    auto operator=(queue&&) noexcept -> queue&      = default;
    ~queue() noexcept                               = default;

    [[nodiscard]] auto queue_family_index() const noexcept -> std::uint32_t {
        return queue_family_index_;
    }

    [[nodiscard]] auto get_present_executor() noexcept -> present_executor {
        return present_executor{get()};
    }

    [[nodiscard]] auto get_submit_executor() noexcept -> submit_executor {
        return submit_executor{get()};
    }

    [[nodiscard]] auto get() noexcept -> VkQueue_T& { return raw_; }

  private:
    explicit queue(VkQueue_T& raw, const std::uint32_t queue_family_index) noexcept
        : raw_{std::move(raw)}, queue_family_index_{queue_family_index} {}

    raii::queue   raw_;
    std::uint32_t queue_family_index_;
};

inline auto device::get_queue(const std::uint32_t queue_family_index) noexcept -> queue {
    auto* raw = static_cast<VkQueue_T*>(nullptr);
    vkGetDeviceQueue(&get(), queue_family_index, 0, &raw);

    assert(raw != nullptr);
    return queue{*raw, queue_family_index};
}
}  // namespace hgn