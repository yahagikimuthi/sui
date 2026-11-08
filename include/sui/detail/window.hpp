#pragma once

#include <expected>
#include <string_view>
#include <utility>

#include <kgm/enums.hpp>
#include <kgm/kgm.hpp>
#include <kgm/window.hpp>

#include "sui/detail/camera.hpp"
#include "sui/detail/input.hpp"
#include "sui/error.hpp"

namespace sui::detail {
class window final {
  public:
    [[nodiscard]] static auto make(
        const std::uint32_t width, const std::uint32_t height, const std::string_view title
    ) noexcept -> std::expected<window, error> {
        if (not kgm::init()) return make_error(runtime_error, "Failed to init GLFW.");

        auto raw = kgm::get_window_creator()
                       .hint(kgm::window_hint::client_api, kgm::window_value::no_api)
                       .extent(width, height)
                       .title(title)
                       .create();

        if (not raw) return make_error(runtime_error, "Failed to make window.");

        return window{std::move(*raw)};
    }

    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept
        : raw_{std::move(other.raw_)}, camera_{other.camera_}, input_{other.input_} {
        reset();
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        raw_    = std::move(other.raw_);
        camera_ = other.camera_;
        input_  = other.input_;

        reset();
        return *this;
    }

    ~window() noexcept = default;

    [[nodiscard]] auto is_open() noexcept -> bool { return not raw_.should_close(); }

    [[nodiscard]] auto get_key_action(const kgm::key key) noexcept -> action {
        return input_.key.get_action(key);
    }

    void update_input() noexcept { input_.update(); }

    [[nodiscard]] auto get() noexcept -> kgm::window& { return raw_; }

  private:
    explicit window(kgm::window&& raw) noexcept : raw_{std::move(raw)} { reset(); }

    void reset() noexcept {
        raw_.set_key_callback([this](kgm::key key, kgm::action act) noexcept -> void {
            input_.key.callback(key, act == kgm::action::press);
        });
    }

    kgm::window raw_;
    camera      camera_;
    input       input_;
};
}  // namespace sui::detail