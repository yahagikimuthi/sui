#pragma once

#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <utility>

#include <hgn/app_instance_info.hpp>
#include <hgn/enums.hpp>
#include <hgn/instance.hpp>

namespace kng {
using hgn::try_enumerate_instance_layer_properties;

class instance final {
  public:
    explicit instance(hgn::instance& native) noexcept : native_{native} {}

    instance(const instance&) noexcept = delete;

    auto operator=(const instance&) noexcept -> instance& = delete;

    instance(instance&& other) noexcept : native_{std::exchange(other.native_, std::nullopt)} {}

    auto operator=(instance&& other) noexcept -> instance& {
        if (this == &other) return *this;

        destroy();

        native_ = std::exchange(other.native_, std::nullopt);

        return *this;
    }

    ~instance() noexcept { destroy(); }

  private:
    void destroy() noexcept {
        if (native_) hgn::destroy_instance(*native_);
    }

    std::optional<hgn::instance&> native_;
};

class instance_builder final {
  public:
    explicit instance_builder() noexcept = default;

    auto app_name(const std::string_view name) noexcept -> instance_builder& {
        app_info_.application_name(name);
        return *this;
    }

    auto app_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept -> instance_builder& {
        app_info_.application_version(major, minor, patch);
        return *this;
    }

    auto engine_name(const std::string_view name) noexcept -> instance_builder& {
        app_info_.engine_name(name);
        return *this;
    }

    auto engine_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept -> instance_builder& {
        app_info_.engine_version(major, minor, patch);
        return *this;
    }

    auto api_version(
        const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch
    ) noexcept -> instance_builder& {
        app_info_.api_version(major, minor, patch);
        return *this;
    }

    auto validation_layers(const std::span<const std::string_view> layers) noexcept
        -> instance_builder& {
        inst_info_.layers(layers);
        return *this;
    }

    auto extensions(const std::span<const std::string_view> vec) noexcept -> instance_builder& {
        inst_info_.extensions(vec);
        return *this;
    }

    auto try_build() noexcept -> std::expected<instance, hgn::result> {
        inst_info_.app_info(app_info_);

        const auto res = hgn::try_make_instance(inst_info_);
        if (not res) return std::unexpected{res.error()};
        return instance{*res};
    }

  private:
    hgn::application_info     app_info_;
    hgn::instance_create_info inst_info_;
};
}  // namespace kng