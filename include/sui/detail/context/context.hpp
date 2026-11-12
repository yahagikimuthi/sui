#pragma once

#include <cstdint>
#include <exception>
#include <expected>
#include <span>
#include <utility>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <kgm/window.hpp>

#include "sui/detail/context/command.hpp"
#include "sui/detail/context/display.hpp"
#include "sui/detail/context/hardware.hpp"
#include "sui/detail/context/renderer.hpp"
#include "sui/detail/others.hpp"
#include "sui/error.hpp"
#include "sui/vertex.hpp"

namespace sui::detail {
class context final {
  public:
    [[nodiscard]] static auto make(kgm::window& win) noexcept -> std::expected<context, error> {
        auto hard = hardware::make(win);
        if (not hard) return make_error(runtime_error, "Failed to make hardware.", hard.error());

        auto display = display::make(*hard);
        if (not display)
            return make_error(runtime_error, "Failed to make display class.", display.error());

        auto cmd = command::make(*hard);
        if (not cmd) return make_error(runtime_error, "Failed to make command.", cmd.error());

        auto render = renderer::make(*hard, display->swapchain().image_format(), cmd->buffer());
        if (not render)
            return make_error(runtime_error, "Failed to make renderer.", render.error());

        return context{std::move(*hard), std::move(*display), std::move(*cmd), std::move(*render)};
    }

    context(const context&) noexcept                    = delete;
    auto operator=(const context&) noexcept -> context& = delete;

    context(context&& other) noexcept
        : hard_{std::move(other.hard_)},
          display_{std::move(other.display_)},
          cmd_{std::move(other.cmd_)},
          render_{std::move(other.render_)},
          is_valid_{std::exchange(other.is_valid_, false)} {}

    auto operator=(context&& other) noexcept -> context& {
        if (this == &other) return *this;

        destroy();

        hard_     = std::move(other.hard_);
        display_  = std::move(other.display_);
        cmd_      = std::move(other.cmd_);
        render_   = std::move(other.render_);
        is_valid_ = std::exchange(other.is_valid_, false);
        return *this;
    }

    ~context() noexcept { destroy(); }

    [[nodiscard]] auto begin_rendering(const uniform_buffer_object& ubo) noexcept -> bool {
        // GPUの待機処理
        if (not hard_.in_flight_fence().wait()) return fail("Failed to wait fence.\n");
        if (not hard_.in_flight_fence().reset()) return fail("Failed to reset fence.\n");

        // 今回の描画で使用する画面を選択
        const auto image_index =
            display_.swapchain().acquire_next_image(hard_.image_available_semaphore());
        if (not image_index) return fail("Failed to acquire next image.\n");
        image_index_ = *image_index;

        // コマンドバッファの記録を開始
        if (not cmd_.buffer().begin()) return fail("Failed to begin command buffer.\n");

        // 画面の状態遷移をする
        // 最初の画像は中身が不定な状態なので、これから色を書き込む状態として設定をする
        cmd_.buffer()
            .get_image_memory_barrier_executor()
            .layout(hgn::image_layout::undefined, hgn::image_layout::color_attachment_optimal)
            .image(display_.get_image(image_index_))
            .barrier();

        // ダイナミックレンダリングの開始
        cmd_.buffer()
            .get_begin_rendering_executor()
            .image_view(display_.get_view(image_index_))
            .clear_color(0.1f, 0.2f, 0.4f, 1.f)
            .extent(800, 600)
            .begin();

        // 使用するバッファとパイプラインのバインド
        cmd_.buffer().bind_pipeline(render_.pipeline());
        cmd_.buffer().bind_vertex_buffer(render_.vbo());
        cmd_.buffer().bind_index_buffer(render_.ibo());
        cmd_.buffer().bind_descriptor_set(render_.pipeline_layout(), render_.descriptor_set());

        // ビューポートとシーザーの設定
        cmd_.buffer().set_viewport(800, 600);
        cmd_.buffer().set_scissor(800, 600);

        render_.ubo().cpy(ubo);

        return true;
    }

    [[nodiscard]] auto draw_texture() noexcept -> bool {
        // GPUの待機処理
        if (not hard_.in_flight_fence().wait()) return fail("Failed to wait fence.\n");
        if (not hard_.in_flight_fence().reset()) return fail("Failed to reset fence.\n");

        // 今回の描画で使用する画面を選択
        const auto image_index =
            display_.swapchain().acquire_next_image(hard_.image_available_semaphore());
        if (not image_index) return fail("Failed to acquire next image.\n");
        image_index_ = *image_index;

        // コマンドバッファの記録を開始
        if (not cmd_.buffer().begin()) return fail("Failed to begin command buffer.\n");

        // 画面の状態遷移をする
        // 最初の画像は中身が不定な状態なので、これから色を書き込む状態として設定をする
        cmd_.buffer()
            .get_image_memory_barrier_executor()
            .layout(hgn::image_layout::undefined, hgn::image_layout::color_attachment_optimal)
            .image(display_.get_image(image_index_))
            .barrier();

        // ダイナミックレンダリングの開始
        cmd_.buffer()
            .get_begin_rendering_executor()
            .image_view(display_.get_view(image_index_))
            .clear_color(0.1f, 0.2f, 0.4f, 1.f)
            .extent(800, 600)
            .begin();

        // 使用するバッファとパイプラインのバインド
        cmd_.buffer().bind_pipeline(render_.texture().pipeline());
        cmd_.buffer().bind_vertex_buffer(render_.vbo());
        cmd_.buffer().bind_index_buffer(render_.ibo());
        cmd_.buffer().bind_descriptor_set(
            render_.texture().pipeline_layout(), render_.texture().descriptor_set()
        );

        // ビューポートとシーザーの設定
        cmd_.buffer().set_viewport(800, 600);
        cmd_.buffer().set_scissor(800, 600);
        render_.ubo().cpy(uniform_buffer_object{});

        const std::vector<uv_vertex> vertices = {
            // 頂点0: 左下
            {{-0.5f, -0.5f}, {255, 255, 255, 255}, {0.0f, 0.0f}},
            // 頂点1: 右下
            {{0.5f, -0.5f}, {255, 255, 255, 255}, {1.0f, 0.0f}},
            // 頂点2: 右上
            {{0.5f, 0.5f}, {255, 255, 255, 255}, {1.0f, 1.0f}},
            // 頂点3: 左上
            {{-0.5f, 0.5f}, {255, 255, 255, 255}, {0.0f, 1.0f}}
        };

        // 2枚の三角形で四角形を構成するインデックス配列
        const std::vector<uint32_t> indices = {
            0,
            1,
            2,  // 1枚目の三角形
            2,
            3,
            0  // 2枚目の三角形
        };

        render_.ibo().cpy(indices);
        render_.vbo().cpy(vertices);
        cmd_.buffer().draw_indexed(static_cast<std::uint32_t>(indices.size()), 1);

        cmd_.buffer().end_rendering();

        // 画面の状態を映し出す形式へと設定変更
        cmd_.buffer()
            .get_image_memory_barrier_executor()
            .layout(hgn::image_layout::color_attachment_optimal, hgn::image_layout::present_src_khr)
            .image(display_.get_image(image_index_))
            .barrier();

        // コマンドバッファの記録終了
        if (not cmd_.buffer().end()) return fail("Failed to end command buffer.\n");

        // 記録したコマンドバッファをGPUのキューに提出
        // このとき、画像が準備できるのを待つセマフォと書き終わったらシグナルを送るセマフォの連動を行う
        if (not hard_.graphics_queue()
                    .get_submitter()
                    .command_buffer(cmd_.buffer())
                    .wait_semaphore(hard_.image_available_semaphore())
                    .signal_semaphore(hard_.render_finished_semaphore())
                    .submit(hard_.in_flight_fence()))
            return fail("Failed to submit.\n");

        // 画面への表示
        const auto res = hard_.present_queue()
                             .get_present_executor()
                             .image_index(image_index_)
                             .semaphore(hard_.render_finished_semaphore())
                             .swapchain(display_.swapchain())
                             .present();
        if (res) return true;

        const auto code = res.error();
        if (code == hgn::result::error_out_of_date_khr or code == hgn::result::suboptimal_khr) {
            // Resize
        }
        return fail("Resize is occurring.\n");

        return true;
    }

    void render(const std::span<const vertex> vertices) noexcept {
        // 頂点データをバッファに書き込み
        render_.vbo().cpy(vertices);

        // コマンドバッファにポリゴンの描画命令を記録
        cmd_.buffer().draw(static_cast<std::uint32_t>(vertices.size()), 1);
    }

    void render(
        const std::span<const vertex> vertices, const std::span<const std::uint32_t> indices
    ) noexcept {
        render_.ibo().cpy(indices);
        render_.vbo().cpy(vertices);
        auto ubo  = uniform_buffer_object{};
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
        render_.ubo().cpy(ubo);
        cmd_.buffer().draw_indexed(static_cast<std::uint32_t>(indices.size()), 1);
    }

    [[nodiscard]] auto end_rendering() noexcept -> bool {
        // ダイナミックレンダリングの終了
        cmd_.buffer().end_rendering();

        // 画面の状態を映し出す形式へと設定変更
        cmd_.buffer()
            .get_image_memory_barrier_executor()
            .layout(hgn::image_layout::color_attachment_optimal, hgn::image_layout::present_src_khr)
            .image(display_.get_image(image_index_))
            .barrier();

        // コマンドバッファの記録終了
        if (not cmd_.buffer().end()) return fail("Failed to end command buffer.\n");

        // 記録したコマンドバッファをGPUのキューに提出
        // このとき、画像が準備できるのを待つセマフォと書き終わったらシグナルを送るセマフォの連動を行う
        if (not hard_.graphics_queue()
                    .get_submitter()
                    .command_buffer(cmd_.buffer())
                    .wait_semaphore(hard_.image_available_semaphore())
                    .signal_semaphore(hard_.render_finished_semaphore())
                    .submit(hard_.in_flight_fence()))
            return fail("Failed to submit.\n");

        // 画面への表示
        const auto res = hard_.present_queue()
                             .get_present_executor()
                             .image_index(image_index_)
                             .semaphore(hard_.render_finished_semaphore())
                             .swapchain(display_.swapchain())
                             .present();
        if (res) return true;

        const auto code = res.error();
        if (code == hgn::result::error_out_of_date_khr or code == hgn::result::suboptimal_khr) {
            // Resize
        }
        return fail("Resize is occurring.\n");
    }

  private:
    explicit context(hardware&& hard, display&& display, command&& cmd, renderer&& render) noexcept
        : hard_{std::move(hard)},
          display_{std::move(display)},
          cmd_{std::move(cmd)},
          render_{std::move(render)} {}

    void destroy() noexcept {
        if (not is_valid_) return;
        if (not hard_.device().wait_idle()) std::terminate();
    }

    hardware      hard_;
    display       display_;
    command       cmd_;
    renderer      render_;
    std::uint32_t image_index_{};
    bool          is_valid_{true};
};
}  // namespace sui::detail