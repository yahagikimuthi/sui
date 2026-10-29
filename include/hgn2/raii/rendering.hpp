#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

namespace hgn::raii {
class pipeline_layout final {
  public:
    [[nodiscard]] explicit pipeline_layout(
        VkPipelineLayout_T&& raw, VkDevice_T& dev  // NOLINT
    ) noexcept
        : raw_{raw}, dev_{std::ref(dev)} {}

    pipeline_layout(const pipeline_layout&) noexcept                    = delete;
    auto operator=(const pipeline_layout&) noexcept -> pipeline_layout& = delete;

    pipeline_layout(pipeline_layout&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(pipeline_layout&& other) noexcept -> pipeline_layout& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~pipeline_layout() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkPipelineLayout_T& { return *raw_; }

    [[nodiscard]] operator VkPipelineLayout_T&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyPipelineLayout(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkPipelineLayout_T&> raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};

class pipeline final {
  public:
    pipeline(const pipeline&) noexcept                    = delete;
    auto operator=(const pipeline&) noexcept -> pipeline& = delete;

    pipeline(pipeline&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(pipeline&& other) noexcept -> pipeline& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~pipeline() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkPipeline_T& { return *raw_; }

    [[nodiscard]] operator VkPipeline_T&() noexcept { return get(); }

  private:
    explicit pipeline(VkPipeline_T& raw, VkDevice_T& dev) noexcept
        : raw_{raw}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (raw_) vkDestroyPipeline(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkPipeline_T&>       raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};

class shader_module final {
  public:
    [[nodiscard]] explicit shader_module(
        VkShaderModule_T&& native, VkDevice_T& dev  // NOLINT
    ) noexcept
        : raw_{native}, dev_{std::ref(dev)} {}

    shader_module(const shader_module&) noexcept                    = delete;
    auto operator=(const shader_module&) noexcept -> shader_module& = delete;

    shader_module(shader_module&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(shader_module&& other) noexcept -> shader_module& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~shader_module() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> VkShaderModule_T& { return *raw_; }

    [[nodiscard]] operator VkShaderModule_T&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyShaderModule(&dev_.get(), &get(), nullptr);
    }

    std::optional<VkShaderModule_T&>   raw_;
    std::reference_wrapper<VkDevice_T> dev_;
};
}  // namespace hgn::raii