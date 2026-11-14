#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <ranges>
#include <vector>

#include <hgn/enums.hpp>
#include <hgn/image.hpp>
#include <hgn/structs.hpp>
#include <hgn/swapchain.hpp>

#include "sui/detail/context/hardware.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class display final {
  public:
    [[nodiscard]] static auto make(hardware& hard) noexcept -> std::expected<display, error> {
        auto& physical = hard.physical();
        auto& surface  = hard.surface();

        const auto capabilities = physical.get_surface_capabilities(surface);
        if (not capabilities)
            return make_error(runtime_error, "Failed to get surface capabilities.");

        // 解像度の決定
        auto extent = capabilities->current_extent();
        // maxでない場合は固定解像度であるため何もしない
        if (extent.width == std::numeric_limits<std::uint32_t>::max()) {
            // 動的解像度の場合は、clampする
            extent.width = std::clamp(
                800u, capabilities->min_image_extent().width, capabilities->max_image_extent().width
            );
            extent.height = std::clamp(
                600u,
                capabilities->min_image_extent().height,
                capabilities->max_image_extent().height
            );
        }

        // swapchain内の画像数を決定(通常はmin+1に設定)
        auto image_count = capabilities->min_image_count() + 1;
        if (capabilities->max_image_count() > 0 and image_count > capabilities->max_image_count()) {
            image_count = capabilities->max_image_count();
        }

        // SurfaceFormatとは、Swapchainが生成するImageのピクセル形式と色空間の組み合わせ
        // GPUが計算したデータをモニターが正しく解釈するため必要
        const auto formats = physical.get_surface_formats(surface);
        if (not formats or formats->empty())
            return make_error(runtime_error, "Failed to get surface formats.");

        auto aligned_formats =
            *formats |
            std::views::filter([](const hgn::surface_format_khr format) noexcept -> bool {
                // 条件式右辺の意味
                // 1.色データがRGBA各8Bにより構成され、sRGB色空間に対応すること
                // 2.標準ダイナミックレンジモニタで一般的な色空間であること
                return format.format() == hgn::format::b8g8r8a8_srgb and
                       format.color_space() == hgn::color_space_khr::srgb_nonlinear;
            });
        const auto surface_format =
            aligned_formats.empty() ? formats->front() : aligned_formats.front();

        // PresentModeとは、計算が完了した画像をどのように画面(フロントバッファ)に転送/表示するかを決める仕組み
        // つまり、計算が終了した後に実際にいつ、どう画面に映すか(映さないか)を決める。
        // 垂直動機や遅延についての設定が中心
        const auto present_modes = physical.get_surface_present_modes(surface);
        if (not present_modes)
            return make_error(runtime_error, "Failed to get surface present modes.");

        auto aligned_modes =
            *present_modes |
            std::views::filter([](const hgn::present_mode_khr mode) noexcept -> bool {
                // mailboxは低遅延かつ滑らかなモード
                return mode == hgn::present_mode_khr::mailbox;
            });

        // mailboxが存在しなければサポートが義務付けされているfifoを採用
        // 垂直同期期間のタイミングに合わせて順番に画面表示する
        // ティアリングが発生しない
        // キューがいっぱいになると描画が待たされるため遅延が発生する可能性がある
        const auto present_mode =
            aligned_modes.empty() ? hgn::present_mode_khr::fifo : aligned_modes.front();
        // なお、他にも同期を待たず直ちに画面へ反映するimmediateなどがある。

        const auto families     = std::array{hard.graphics_family(), hard.present_family()};
        const bool is_duplicate = families[0] == families[1];
        auto       creator      = hard.device()
                           .get_swapchain_creator()
                           .surface(surface)
                           .min_image_count(image_count)
                           .image_format(surface_format.format())
                           .image_color_space(surface_format.color_space())
                           .image_extent(extent)
                           .pre_transform(capabilities->current_transform())
                           .present_mode(present_mode);
        if (is_duplicate) {
            creator.image_sharing_mode(hgn::sharing_mode::exclusive);
        } else {
            creator.image_sharing_mode(hgn::sharing_mode::concurrent).queue_families(families);
        }
        auto swapchain = creator.create();
        if (not swapchain) return make_error(runtime_error, "Failed to make swapchain.");

        auto images = swapchain->get_images();
        if (not images) return make_error(runtime_error, "Failed to get images.");

        auto views = std::vector<hgn::image_view>{};
        views.reserve(images->size());

        for (auto& image : *images) {
            auto view = hard.device()
                            .get_image_view_creator()
                            .image(image)
                            .format(swapchain->image_format())
                            .create();
            if (not view) return make_error(runtime_error, "Failed to make image view.");

            views.emplace_back(std::move(*view));
        }

        return display{std::move(*swapchain), std::move(*images), std::move(views)};
    }

    [[nodiscard]] auto get_image(const std::size_t i) noexcept -> hgn::borrowed_image& {
        return images_[i];
    }
    [[nodiscard]] auto get_view(const std::size_t i) noexcept -> hgn::image_view& {
        return views_[i];
    }

    [[nodiscard]] auto swapchain() noexcept -> hgn::swapchain_khr& { return swapchain_; }

  private:
    display(
        hgn::swapchain_khr&&               swapchain,
        std::vector<hgn::borrowed_image>&& images,
        std::vector<hgn::image_view>&&     views
    ) noexcept
        : swapchain_{std::move(swapchain)}, images_{std::move(images)}, views_{std::move(views)} {}

    hgn::swapchain_khr               swapchain_;
    std::vector<hgn::borrowed_image> images_;
    std::vector<hgn::image_view>     views_;
};
}  // namespace sui::detail