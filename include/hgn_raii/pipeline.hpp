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
    ) noexcept -> std::expected<raii::pipeline_layout, hgn::result> {
        auto res = hgn::try_make_pipeline_layout(dev, info);
        if (not res) return std::unexpected{res.error()};

        return raii::pipeline_layout{*res, dev};
    }

    pipeline_layout(const raii::pipeline_layout&) noexcept                          = delete;
    auto operator=(const raii::pipeline_layout&) noexcept -> raii::pipeline_layout& = delete;

    pipeline_layout(raii::pipeline_layout&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(raii::pipeline_layout&& other) noexcept -> raii::pipeline_layout& {
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
) noexcept -> std::expected<raii::pipeline_layout, hgn::result> {
    return raii::pipeline_layout::try_make(info, dev);
}

class graphics_pipeline final {
  public:
    [[nodiscard]] static auto try_make(
        const hgn::graphics_pipeline_create_info& info, hgn::device& dev
    ) noexcept -> std::expected<raii::graphics_pipeline, hgn::result> {
        auto res = hgn::try_make_graphics_pipeline(dev, info);
        if (not res) return std::unexpected{res.error()};

        return raii::graphics_pipeline{*res, dev};
    }

    graphics_pipeline(const raii::graphics_pipeline&) noexcept                          = delete;
    auto operator=(const raii::graphics_pipeline&) noexcept -> raii::graphics_pipeline& = delete;

    graphics_pipeline(raii::graphics_pipeline&& other) noexcept
        : native_{std::exchange(other.native_, std::nullopt)}, dev_{other.dev_} {}

    auto operator=(raii::graphics_pipeline&& other) noexcept -> raii::graphics_pipeline& {
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
) noexcept -> std::expected<raii::graphics_pipeline, hgn::result> {
    return raii::graphics_pipeline::try_make(info, dev);
}
}  // namespace hgn::raii