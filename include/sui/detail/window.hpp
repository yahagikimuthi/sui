#pragma once

#include <expected>
#include <optional>
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

        auto raw = kgm::window_creator{}.extent(width, height).title(title).create();
        if (not raw) return make_error(runtime_error, "Failed to make window.");

        return window{std::move(*raw)};
    }

    window(const window&) noexcept                    = delete;
    auto operator=(const window&) noexcept -> window& = delete;

    window(window&& other) noexcept
        : raw_{std::move(other.raw_)},
          camera_{other.camera_},
          input_{other.input_},
          is_valid_{std::exchange(other.is_valid_, false)} {
        reset();
    }

    auto operator=(window&& other) noexcept -> window& {
        if (this == &other) return *this;

        raw_      = std::move(other.raw_);
        camera_   = other.camera_;
        input_    = other.input_;
        is_valid_ = std::exchange(other.is_valid_, false);

        reset();
        return *this;
    }

    ~window() noexcept {
        if (is_valid_) kgm::terminate();
    }

    [[nodiscard]] auto is_open() noexcept -> bool { return not raw_.should_close(); }

    [[nodiscard]] auto input() noexcept -> input& { return input_; }

    [[nodiscard]] auto camera() noexcept -> camera& { return camera_; }
    [[nodiscard]] auto camera() const noexcept -> const class camera& { return camera_; }

    [[nodiscard]] auto get() noexcept -> kgm::window& { return raw_; }

  private:
    explicit window(kgm::window&& raw) noexcept : raw_{std::move(raw)} { reset(); }

    void reset() noexcept {
        raw_.set_key_callback([this](kgm::key key, kgm::action action) noexcept -> void {
            input_.keys.set(key, action);
        });
    }

    kgm::window  raw_;
    class camera camera_;
    struct input input_;
    bool         is_valid_{true};
};
}  // namespace sui::detail