#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raw.hpp"

namespace hgn::raii {
class pipeline_layout final {
  public:
    [[nodiscard]] explicit pipeline_layout(
        raw::pipeline_layout&& raw, raw::device& dev  // NOLINT
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

    [[nodiscard]] auto get() noexcept -> raw::pipeline_layout& { return *raw_; }

    [[nodiscard]] operator raw::pipeline_layout&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyPipelineLayout(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::pipeline_layout&> raw_;
    std::reference_wrapper<raw::device>  dev_;
};

class pipeline final {
  public:
    [[nodiscard]] explicit pipeline(raw::pipeline&& raw, raw::device& dev) noexcept  // NOLINT
        : raw_{raw}, dev_{std::ref(dev)} {}

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

    [[nodiscard]] auto get() noexcept -> raw::pipeline& { return *raw_; }

    [[nodiscard]] operator raw::pipeline&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyPipeline(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::pipeline&>       raw_;
    std::reference_wrapper<raw::device> dev_;
};

class shader_module final {
  public:
    [[nodiscard]] explicit shader_module(
        raw::shader_module&& native, raw::device& dev  // NOLINT
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

    [[nodiscard]] auto get() noexcept -> raw::shader_module& { return *raw_; }

    [[nodiscard]] operator raw::shader_module&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyShaderModule(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::shader_module&>  raw_;
    std::reference_wrapper<raw::device> dev_;
};
}  // namespace hgn::raii