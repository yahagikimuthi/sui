#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <vulkan/vulkan.h>

#include "hgn/raw/raw.hpp"

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

class descriptor_set_layout final {
  public:
    explicit descriptor_set_layout(
        raw::descriptor_set_layout&& raw, raw::device& dev  // NOLINT
    ) noexcept
        : raw_{raw}, dev_{std::ref(dev)} {}

    descriptor_set_layout(const descriptor_set_layout&) noexcept                    = delete;
    auto operator=(const descriptor_set_layout&) noexcept -> descriptor_set_layout& = delete;

    descriptor_set_layout(descriptor_set_layout&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(descriptor_set_layout&& other) noexcept -> descriptor_set_layout& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~descriptor_set_layout() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> raw::descriptor_set_layout& { return *raw_; }

    [[nodiscard]] operator raw::descriptor_set_layout&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyDescriptorSetLayout(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::descriptor_set_layout&> raw_;
    std::reference_wrapper<raw::device>        dev_;
};

class descriptor_pool final {
  public:
    descriptor_pool(raw::descriptor_pool&& raw, raw::device& dev) noexcept  // NOLINT
        : raw_{raw}, dev_{std::ref(dev)} {}

    descriptor_pool(const descriptor_pool&) noexcept                    = delete;
    auto operator=(const descriptor_pool&) noexcept -> descriptor_pool& = delete;

    descriptor_pool(descriptor_pool&& other) noexcept
        : raw_{std::exchange(other.raw_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(descriptor_pool&& other) noexcept -> descriptor_pool& {
        if (this == &other) return *this;

        destroy();

        raw_ = std::exchange(other.raw_, std::nullopt);
        dev_ = other.dev_;
        return *this;
    }

    ~descriptor_pool() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> raw::descriptor_pool& { return *raw_; }

    [[nodiscard]] operator raw::descriptor_pool&() noexcept { return get(); }

  private:
    void destroy() noexcept {
        if (raw_) vkDestroyDescriptorPool(&dev_.get(), &get(), nullptr);
    }

    std::optional<raw::descriptor_pool&> raw_;
    std::reference_wrapper<raw::device>  dev_;
};

class descriptor_set final {
  public:
    [[nodiscard]] descriptor_set(raw::descriptor_set& raw) noexcept : raw_{raw} {}

    [[nodiscard]] auto get() noexcept -> raw::descriptor_set& { return raw_; }

    [[nodiscard]] operator raw::descriptor_set&() noexcept { return get(); }

  private:
    std::reference_wrapper<raw::descriptor_set> raw_;
};
}  // namespace hgn::raii