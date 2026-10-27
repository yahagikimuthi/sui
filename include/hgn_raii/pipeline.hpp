#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <utility>

#include <hgn/declaration.hpp>
#include <hgn/enums.hpp>
#include <hgn/pipeline.hpp>

namespace hgn::raii {
class pipeline_layout final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::pipeline_layout_create_info& info, hgn::device& dev
    ) noexcept -> std::expected<pipeline_layout, hgn::result> {
        auto res = hgn::try_make_pipeline_layout(dev, info);
        if (not res) return std::unexpected{res.error()};

        return pipeline_layout{*res, dev};
    }

    pipeline_layout(const pipeline_layout&) noexcept                    = delete;
    auto operator=(const pipeline_layout&) noexcept -> pipeline_layout& = delete;

    pipeline_layout(pipeline_layout&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(pipeline_layout&& other) noexcept -> pipeline_layout& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;
        return *this;
    }

    ~pipeline_layout() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::pipeline_layout& { return *native_; }

    [[nodiscard]] operator hgn::pipeline_layout&() noexcept { return get(); }

  private:
    explicit pipeline_layout(hgn::pipeline_layout& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_pipeline_layout(dev_, get());
    }

    std::optional<hgn::pipeline_layout&> native_;
    std::reference_wrapper<hgn::device>  dev_;
};

[[nodiscard]] inline auto try_make_pipeline_layout(
    const hgn::pipeline_layout_create_info& info, hgn::device& dev
) noexcept -> std::expected<pipeline_layout, hgn::result> {
    return pipeline_layout::try_make(info, dev);
}

class graphics_pipeline final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::graphics_pipeline_create_info& info, hgn::device& dev
    ) noexcept -> std::expected<graphics_pipeline, hgn::result> {
        auto res = hgn::try_make_graphics_pipeline(dev, info);
        if (not res) return std::unexpected{res.error()};

        return graphics_pipeline{*res, dev};
    }

    graphics_pipeline(const graphics_pipeline&) noexcept                    = delete;
    auto operator=(const graphics_pipeline&) noexcept -> graphics_pipeline& = delete;

    graphics_pipeline(graphics_pipeline&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(graphics_pipeline&& other) noexcept -> graphics_pipeline& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);
        dev_    = other.dev_;
        return *this;
    }

    ~graphics_pipeline() noexcept { destroy(); }

    [[nodiscard]] auto get() noexcept -> hgn::pipeline& { return *native_; }

    [[nodiscard]] operator hgn::pipeline&() noexcept { return get(); }

  private:
    explicit graphics_pipeline(hgn::pipeline& native, hgn::device& dev) noexcept
        : native_{native}, dev_{std::ref(dev)} {}

    void destroy() noexcept {
        if (native_) hgn::destroy_pipeline(dev_, get());
    }

    std::optional<hgn::pipeline&>       native_;
    std::reference_wrapper<hgn::device> dev_;
};

[[nodiscard]] inline auto try_make_graphics_pipeline(
    const hgn::graphics_pipeline_create_info& info, hgn::device& dev
) noexcept -> std::expected<graphics_pipeline, hgn::result> {
    return graphics_pipeline::try_make(info, dev);
}
}  // namespace hgn::raii